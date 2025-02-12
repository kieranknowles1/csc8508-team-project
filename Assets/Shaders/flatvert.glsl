#version 400 core

layout(location = 1) in vec4 colour;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec4 objectColour = vec4(1,1,1,1);
in vec3 position;

out Vertex {
    vec4 color;
    vec3 worldPos;
    } OUT;

void main() {
    gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(position, 1.0);
    OUT.color = objectColour;
    }