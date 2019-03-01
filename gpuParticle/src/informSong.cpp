//
//  informSong.cpp
//  gpuParticle
//
//  Created by 引田祐樹 on 2019/03/01.
//

#include "informSong.hpp"

informSong::informSong(string _name,string _song,string _popular){
    name=_name;
    
    int fontSize=40;
    fontArtist.load("/Users/amaimon/Library/Fonts/Montserrat-Bold.ttf",fontSize);
    fontSize=35;
    fontSong.load("/Users/amaimon/Library/Fonts/Montserrat-Bold.ttf",fontSize);
    fontSize=35;
    fontPop.load("/Users/amaimon/Library/Fonts/Montserrat-Bold.ttf",fontSize);

    
    ofFbo fbo;
    //アルファチャンネルのなしのdefultの設定
    fbo.allocate(ofGetWidth(), ofGetHeight());
    fbo.begin();
    //fboはprocessing同様、backgroundがautoではないため取得
    ofClear(0);
    fontArtist.drawString(name, ofGetWidth()*4/5-fontArtist.stringWidth(name)/2, fontArtist.stringHeight(name)*2);
    fbo.end();
    
    ofPixels pixels;
    fbo.readToPixels(pixels);
    glPointSize(0.5);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINES);
    //fontの間隔
    fontSize=2;
    int index=ofRandom(100);
    for (int x=0; x<fbo.getWidth(); x+=fontSize) {
        for (int y=0; y<fbo.getHeight(); y+=fontSize) {
            if(pixels.getColor(x, y)!=ofColor(0,0)){
                
                float dx=x+ofMap(ofNoise(x,y),0,1,-3,3);
                float dy=y+ofMap(ofNoise(x,y),0,1,-3,3);

                mesh.addVertex(vec3(dx,dy,0));
                ofColor color;
                color.setHsb(index, 255*0.6, 255*0.8);
                mesh.addColor(color);
                index+=0.5;
            }
        }
    }
    
    fbo.begin();
    //fboはprocessing同様、backgroundがautoではないため取得
    ofClear(0);
    fontSong.drawString(_song, ofGetWidth()*4/5-fontSong.stringWidth(_song)/2, fontArtist.stringHeight(_name)*2+fontSong.stringHeight(_song)*2);
    fbo.end();
    
    fbo.readToPixels(pixels);
    
    for (int x=0; x<fbo.getWidth(); x+=fontSize) {
        for (int y=0; y<fbo.getHeight(); y+=fontSize) {
            if(pixels.getColor(x, y)!=ofColor(0,0)){
                
                mesh.addVertex(vec3(x,y,0));
                ofColor color;
                color.setHsb(index, 255*0.6, 255*0.8);
                mesh.addColor(color);
                index+=0.5;
            }
        }
    }
    
    fbo.begin();
    //fboはprocessing同様、backgroundがautoではないため取得
    ofClear(0);
    fontPop.drawString(_popular, ofGetWidth()*4/5-fontPop.stringWidth(_popular)/2,fontArtist.stringHeight(_name)*2+ fontSong.stringHeight(_song)*2+fontPop.stringHeight(_popular)*2);
    fbo.end();
    
    fbo.readToPixels(pixels);
    
    fontSize=1;
    
    for (int x=0; x<fbo.getWidth(); x+=fontSize) {
        for (int y=0; y<fbo.getHeight(); y+=fontSize) {
            if(pixels.getColor(x, y)!=ofColor(0,0)){
                
                mesh.addVertex(vec3(x,y,0));
                ofColor color;
                color.setHsb(index, 255*0.6, 255*0.8);
                mesh.addColor(color);
                index+=0.5;
            }
        }
    }
}

void informSong::display(){
    mesh.draw();
    
}


