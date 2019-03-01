#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    soundNum=3;
    
    ofSetFullscreen(true);
    ofSetFrameRate(60);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofBackground(0);
    //ofHideCursor();

    maxNum=2000000;
    
    isAttract=false;
    
    // GUI
    gui.setup();
    gui.setPosition(20, 20);
    gui.add(strength.set("strength", 0.9, 0.0, 1.0));
    gui.add(centerX.set("centerX", ofGetWidth() / 2, 0, ofGetWidth()));
    gui.add(centerY.set("centerY", ofGetHeight() / 2, 0, ofGetHeight()));
    gui.add(centerZ.set("centerZ", ofGetWidth() / 2, 0, ofGetWidth()));
    gui.add(numParticle.set("popular",0,0,maxNum));
    gui.add(x.set("x",0,-1000,1000));
    gui.add(y.set("y",0,-2000,-500));
    gui.add(z.set("z",0,-1000,1000));
    gui.add(dx.set("dx",0,0,1.0));
    gui.add(dy.set("dy",0,0,1.0));
    gui.add(dz.set("dz",0,0,1.0));
    
    width =ofGetWidth();
    height = ofGetHeight();
    mouseFrag = false;
    
    noiseX=ofRandom(10000);
    noiseY=ofRandom(10000);
    noiseZ=ofRandom(10000);
    
    // シェーダを読み込む
    updatePos.load("shaders/passthru.vert", "shaders/posUpdate.frag");
    updateRender.load("shaders/render.vert", "shaders/render.frag");
    
    //fft setup
    fft.setup(pow(2, 9));
    
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
    
    artist[0]="Nakata Yasutaka";
    artist[1]="PERFUME";
    artist[2]="Virtual Riot";
    
    song[0]="Give You More";
    song[1]="If you wanna";
    song[2]="Energy Drink";
    
    dance[0]= 0.651;
    dance[1]= 0.571;
    dance[2]= 0.817;
    
    popular[0]=134821;
    popular[1]=205737;
    popular[2]=663453;
    
    receiver.setup(PORT);
}

//--------------------------------------------------------------
void ofApp::update(){
    
    while(receiver.hasWaitingMessages()){
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        if(m.getAddress()=="changeSound"){
            changeNum(m.getArgAsInt(0));
        }
    }
    
    
    fft.update();
    vector<float> buffer;
    buffer=fft.getBins();
    
    time += 0.01;
    
    // attractorの位置(x, y, z)を音声から計算
    centerX = ofMap(buffer[0], 0, 1, 0, ofGetWidth());
    centerY = ofMap((buffer[5] - abs(sin(time) * 0.1)), 0, 1, ofGetHeight() * 0.50, ofGetHeight() * 0.75);
    centerZ = ofMap(buffer[10], 0, 1, 0, ofGetWidth()) * 15.0;
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
    if(buffer[27]>0.9){
        resetPos();
    }

    //---------------------カメラの位置の移動
    buffer[15]=(floor(100*buffer[15]))/100;
    buffer[20]=(floor(100*buffer[20]))/100;
    buffer[25]=(floor(100*buffer[25]))/100;
    
    noiseX=(floor(100*noiseX))/100;
    noiseY=(floor(100*noiseY))/100;
    noiseZ=(floor(100*noiseZ))/100;
    
    dx=ofMap(buffer[40], 0, 1, 0, 0.2);
    dy=ofMap(buffer[45], 0, 1, 0, 0.2);
    dz=ofMap(buffer[50], 0, 1, 0, 0.2);
    
    if(!isnan(buffer[15])){
        noiseX+=(dx*buffer[15]);
    }
    
    if(!isnan(buffer[20])){
        noiseY+=(dy*buffer[20]);
    }
    
    if(!isnan(buffer[25])){
        noiseZ+=(dz*buffer[25]);
    }
    
    x=ofMap(ofNoise(noiseX), 0, 1, -500, 500);
    y=ofMap(ofNoise(noiseY), 0, 1, -2000,-500);
    z=ofMap(ofNoise(noiseZ), 0, 1, -1000, 1000);
    
    cam.setPosition(x, y, z);
    
    depth=ofMap(buffer[30], 0, 1, 1500, 6000);
    
    if(buffer[24]>0.9){
        changeAttractor();
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    
    cam.begin();
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
    //gui.draw();
    //ofSetColor(255);
    //ofDrawBitmapString("Fps: " + ofToString(ofGetFrameRate()), 15, 15);
    
    if(infoSong.size()>0){
        infoSong[0]->display();
    }
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
void ofApp::changeAttractor(){
    if(isAttract){
        isAttract=false;
    }else{
        isAttract=true;
    }
}

//--------------------------------------------------------------
void ofApp::changeNum(int _num){
    int num=_num;
    if(num>3){
        resetPos();
        return;
    }
    soundNum=num;
    numParticle=popular[soundNum];
    strength=dance[soundNum];
    infoSong.clear();
    infoSong.push_back(new informSong(artist[soundNum],song[soundNum],ofToString(popular[soundNum])));
    resetPos();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    int num=key-49;
    changeNum(num);
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
