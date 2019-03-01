//
//  informSong.hpp
//  gpuParticle
//
//  Created by 引田祐樹 on 2019/03/01.
//

#pragma once
#ifndef informSong_hpp
#define informSong_hpp

#include <stdio.h>
#include "ofMain.h"

using namespace glm;

class informSong {
    
public:
    informSong(string _name,string _song,string popular);
    ~informSong();
    void display();
    
    string name;
    
    ofTrueTypeFont fontArtist;
    ofTrueTypeFont fontSong;
    ofTrueTypeFont fontPop;
    ofVboMesh mesh;
    vector<vec2> locations;
};

#endif /* informSong_hpp */
