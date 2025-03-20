#version 400 core

#define mul(a, b) ((a) * (b))
#include "include/vert/laser.glsl"

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;
uniform float time;

out float depth;

layout(location = 0) in vec3 position;

void main(void) {
    LaserVertIn state;
    state.start = startPosition;
    state.end = endPosition;
    state.thickness = thickness;
    state.time = time;

    LaserVertOut result = doLaserVert(state, position, viewMatrix, projMatrix);
    gl_Position = result.position;
    depth = result.depth;
}
