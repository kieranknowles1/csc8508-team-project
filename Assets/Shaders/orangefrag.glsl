#version 330 core
#include "include/vertex.glh" //because default frag used this

in Vertex IN;

out vec4 fragColor;

void main() {
//adjust the vec4 values until satisfied with shade of Orange
     fragColor = vec4(1.0, 1.0, 0.0, 1.0);
     }

