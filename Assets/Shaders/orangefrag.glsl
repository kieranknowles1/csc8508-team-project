#version 330 core

in Vertex {
    vec4 color;
    vec3 worldPos;
    }IN;

out vec4 fragColor;

void main() {
//adjust the vec4 values until satisfied with shade of Orange
     fragColor = vec4(1.0, 0.5, 0.0, 1.0);
     }

