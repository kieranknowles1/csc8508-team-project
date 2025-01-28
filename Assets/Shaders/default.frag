#version 330 core
#include "include/vertex.glh"

in Vertex IN;

out vec4 fragColor;

void main() {
    // fragColor = IN.color;
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
