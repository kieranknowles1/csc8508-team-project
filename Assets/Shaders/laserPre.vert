#version 400 core


layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out Vertex {
    vec2 TexCoord;
} OUT;

void main(void) {

    OUT.TexCoord = texCoord;
    gl_Position = vec4(position, 1.0);
}
