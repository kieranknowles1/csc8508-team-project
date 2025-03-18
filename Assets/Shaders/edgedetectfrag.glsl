#version 400 core

uniform sampler2D depthTex;
double depthSample(vec2 coord) {
    return texture(depthTex, coord).r;
}
#define mul(a, b) ((a) * (b))
#include "include/post/edgedetect.glsl"

uniform sampler2D sceneTex;

uniform vec2 windowSize;

uniform mat4 inverseProjMatrix;
uniform mat4 inverseViewMatrix;


in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColor;


void main(void) {
    vec4 colour = texture(sceneTex, IN.texCoord); //initially just render scene as is ////////
    vec2 invWinSize = vec2(1.0f / windowSize.x, 1.0f / windowSize.y);
    EdgeDetectMatrices mats;
    mats.invView = inverseViewMatrix;
    mats.invProj = inverseProjMatrix;
    fragColor = doEdgeDetect(colour, IN.texCoord, invWinSize, mats);
 }
