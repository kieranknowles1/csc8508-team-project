#version 330 core

layout(location = 0) in vec3 aPos; // Position from VAO
layout(location = 1) in vec2 aTexCoord; // Texture coordinates from VAO

out vec3 FragPos; // Position to fragment shader
out vec2 TexCoord; // Texture coordinates to fragment shader

uniform mat4 modelMatrix;
uniform mat4 viewProjMatrix;

void main() {
    FragPos = (modelMatrix * vec4(aPos, 1.0)).xyz;
    TexCoord = aTexCoord;
    gl_Position = viewProjMatrix * vec4(FragPos, 1.0);
}