#version 400 core

#define mul(a, b) ((a) * (b))
#include "include/types.h"
#include "include/frag/pointlight.glsl"

uniform sampler2D depthTex; //need the depth texture to get world space position of fragment light covers
uniform sampler2D normTex;

uniform vec2 pixelSize;
uniform vec3 cameraPos;

uniform float lightRadius;
uniform vec3 lightPos;
uniform vec4 lightColour;
uniform float lightIntensity;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void) {
    vec2  texCoord   = vec2(gl_FragCoord.xy * pixelSize);
    vec3 normal   = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
    float depth      = texture(depthTex, texCoord.xy).r;

    LightState state;
    state.radius = lightRadius;
    state.position = lightPos;
    state.color = lightColour;
    state.intensity = lightIntensity;

    LightOutput result = doPointLight(
        state,
        normal,
        cameraPos,
        texCoord,
        depth,
        inverseProjView
    );
    diffuseOutput = result.diffuse;
    specularOutput = result.specular;
}
