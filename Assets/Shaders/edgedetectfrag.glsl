#version 420 core

uniform sampler2D normalTex;
vec3 normalSample(vec2 coord) {
    return texture(normalTex, coord).rgb;
}
#define mul(a, b) ((a) * (b))
#include "include/post/edgeapply.glsl"

uniform sampler2D sceneTex;
uniform vec2 windowSize;


in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColor;

void main(void) {
    vec2 invWinSize = vec2(1.0f / windowSize.x, 1.0f / windowSize.y);
    vec4 colour = texture(sceneTex, IN.texCoord); //initially just render scene as is ////////
    fragColor = doEdgeDetect(colour, IN.texCoord,invWinSize);
 }
