#version 330 core

in Vertex {
    vec4 color;
    vec3 worldPos;
    }IN;

out vec4 fragColor;

void main() {
     fragColor =IN.color;
     }
