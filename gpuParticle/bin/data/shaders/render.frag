#version 150
precision mediump float;

uniform vec2 screen;

in vec4 vColor;
in vec2 vTexCoord;
in vec4 vPosition;

out vec4 vFragColor;

void main() {
    vFragColor = vColor;
}
