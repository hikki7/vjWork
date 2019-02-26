#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFullscreen(true);
    ofSetFrameRate(60);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofBackground(0);

    maxNum=1000000;
    
    // GUI
    gui.setup();
    gui.setPosition(20, 20);
    gui.add(strength.set("strength", 0.9, 0.0, 1.0));
    gui.add(isAttract.setup("Attract", true));
    gui.add(centerX.set("centerX", ofGetWidth() / 2, 0, ofGetWidth()));
    gui.add(centerY.set("centerY", ofGetHeight() / 2, 0, ofGetHeight()));
    gui.add(centerZ.set("centerZ", ofGetWidth() / 2, 0, ofGetWidth()));
    gui.add(numParticle.setup("numParticle",100,0,maxNum));
    
    width =ofGetWidth();
    height = ofGetHeight();
    mouseFrag = false;
    
    

    
    // シェーダを読み込む
    updatePos.load("shaders/passthru.vert", "shaders/posUpdate.frag");
    updateRender.load("shaders/render.vert", "shaders/render.frag");
    
    // 音声の設定
    sound.load("digitalworld.MP3");
    sound.setLoop(true);
    sound.play();
    nBandsToGet = 4;
    fftSmoothed = new float[nBandsToGet];
    for(int i = 0; i < nBandsToGet; i++) {
        fftSmoothed[i] = 0;
    }
    
    //fft setup
    fft.setup(pow(2, 2));
    
    int maxRes=ceil(sqrt(maxNum));
    
    // パーティクルの座標、速度、加速度の保存用FBO
    // RGB32Fの形式で3つのカラーバッファを用意
    posPingPong.allocate(maxRes, maxRes, GL_RGB32F, 3);
    
    resetPos();
    
    // renderFBOを初期化
    renderFBO.allocate(width, height, GL_RGB32F);
    renderFBO.begin();
    ofClear(0, 0, 0, 255);
    renderFBO.end();
    
    
    // 引力を発生する場所を中心に設定
    attractor = ofVec3f(ofGetWidth() / 2, ofGetHeight() / 2, 0);
    
    
    glPointSize(1.0);
    cam.setTarget(ofVec3f(0,ofGetHeight()/2,0));
}

//--------------------------------------------------------------
void ofApp::update(){
    
    fft.update();
    vector<float> buffer;
    buffer=fft.getBins();
    //ofLog()<<buffer.size();
    
    time += 0.01;
    
    ofSoundUpdate();
    volume = ofSoundGetSpectrum(nBandsToGet);
    
    // fftSmoothedに音声のデータ(配列)を格納
    for(int i = 0; i < nBandsToGet; i++) {
        fftSmoothed[i] *= 0.96f;
        if(fftSmoothed[i] < volume[i]) {
            fftSmoothed[i] = volume[i];
        }
    }
    
    // attractorの位置(x, y, z)を音声から計算
    centerX = ofMap(fftSmoothed[0], 0, 1, 0, ofGetWidth());
    centerY = ofMap((fftSmoothed[1] - abs(sin(time) * 0.1)), 0, 1, ofGetHeight() * 0.50, ofGetHeight() * 0.75);
    centerZ = ofMap(fftSmoothed[2], 0, 1, 0, ofGetWidth()) * 20.0;
    attractor = ofVec3f(centerX, centerY, centerZ);
    
    
    // Position PingPong
    posPingPong.dst->begin();
    ofClear(0);
    updatePos.begin();
    updatePos.setUniformTexture("posData", posPingPong.src->getTexture(0), 0); // 位置のテクスチャ
    updatePos.setUniformTexture("velData", posPingPong.src->getTexture(1), 1); // 速度のテクスチャ
    updatePos.setUniformTexture("accData", posPingPong.src->getTexture(2), 2); // 加速度のテクスチャ
    updatePos.setUniform3f("attractor", attractor); // 引力発生地
    updatePos.setUniform1f("strength", strength); // 引力の強度
    updatePos.setUniform1i("isAttract", isAttract); // 引力か反発か
    updatePos.setUniform1f("time", time);
    
    posPingPong.src->draw(0, 0);
    
    updatePos.end();
    posPingPong.dst->end();
    
    // srcとdstを入れ替え
    posPingPong.swap();
    
    //たまにリセットかけたいよ〜
    check=ofRandom(100);
    if(check>98){
        resetPos();
    }
    
    
    
    

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    
    cam.begin();
    depth=ofMap(fftSmoothed[3], 0, 1, 1500, 6000);
    cam.setDistance(depth);
    
    updateRender.begin();
    updateRender.setUniformTexture("posTex", posPingPong.dst->getTexture(), 0);
    updateRender.setUniform1i("resolution", (float)textureRes);
    updateRender.setUniform2f("screen", (float)width, (float)height);
    updateRender.setUniform1f("time", time);
    
    mesh.draw(OF_MESH_WIREFRAME);
    
    updateRender.end();
    
    cam.end();
    
    ofDisableDepthTest();
    
    // デバッグ用
    gui.draw();
    ofSetColor(255);
    ofDrawBitmapString("Fps: " + ofToString(ofGetFrameRate()), 15, 15);
    
    
    
}

//--------------------------------------------------------------
void ofApp::resetPos(){
     
    textureRes = ceil(sqrt(numParticle));
    
    pos.resize(3*textureRes*textureRes);
    
    vel.resize(3*textureRes*textureRes);
    
    acc.resize(3*textureRes*textureRes);
    
    // パーティクルの速度の初期設定
    for(int i = 0; i < numParticle; i++) {
        
        pos[i*3 + 0] = ofRandom(1.0);
        pos[i*3 + 1] = ofRandom(1.0);
        pos[i*3 + 2] = ofRandom(1.0);
        
        vel[i*3 + 0] = 0.0;
        vel[i*3 + 1] = 0.0;
        vel[i*3 + 2] = 0.0;
        
        acc[i*3 + 0] = 0.0;
        acc[i*3 + 1] = 0.0;
        acc[i*3 + 2] = 0.0;
    }
    
    // pingPongBufferに初期値を書き込む
    posPingPong.src->getTexture(0).loadData(pos.data(), textureRes, textureRes, GL_RGB);
    
    // pingPongBufferに初期値を書き込む
    posPingPong.src->getTexture(1).loadData(vel.data(), textureRes, textureRes, GL_RGB);
    
    // pingPongBufferに初期値を書き込む
    posPingPong.src->getTexture(2).loadData(acc.data(), textureRes, textureRes, GL_RGB);
    
    // VBOMeshの初期設定
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_POINTS);
    for(int x = 0; x < textureRes; x++) {
        for(int y = 0; y < textureRes; y++) {
            int i = y * textureRes + x;
            mesh.addVertex(ofVec3f(x, y));
            mesh.addTexCoord(ofVec2f(x, y));
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    resetPos();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}
