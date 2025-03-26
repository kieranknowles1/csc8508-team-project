#version 330 core

layout (location = 0) in vec2 aPos;      // 2D position in normalized device coordinates (NDC)
layout (location = 2) in vec2 aTexCoord; // Texture coordinates

out vec2 texCoord; // Pass texture coordinates to fragment shader
out vec4 Color;    // Pass color to fragment shader

uniform vec2 position; // Normalized screen-space position (0 to 1)
uniform vec2 size;     // UI element size in normalized coordinates
uniform vec4 color;    // UI color (RGBA)

void main() {
    // Convert normalized screen-space position to NDC (-1 to 1)
    vec2 scaledPos = (aPos * size) + position;
    gl_Position = vec4(scaledPos * 2.0 - 1.0, 0.0, 1.0);
    
    texCoord = aTexCoord;
    Color = color;
}