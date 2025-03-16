#version 400 core

uniform mat4 prevViewProjMatrix;
uniform mat4 currViewProjMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out Vertex {
    vec2 correctedTexCoord;
    vec2 originalTexCoord;
} OUT;

void main(void) {

    OUT.correctedTexCoord = texCoord;
    OUT.originalTexCoord = texCoord; 

    gl_Position = vec4(position, 1.0);
}
