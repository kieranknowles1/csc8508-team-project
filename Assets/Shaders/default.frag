#version 330 core

in Vertex {
    vec4 color;
} IN;

out vec4 fragColor;

void main() {
    // fragColor = IN.color;
    fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
