#version 400 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

in vec3 position;
in vec4 colour;

out Vertex {
    vec4 color;
    vec3 worldPos;
    } OUT;

void main() {
    gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
    OUT.color = colour;
    }