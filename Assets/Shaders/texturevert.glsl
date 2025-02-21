#version 330 core//should this be 400?

//uniform mat4 modelMatrix; //basically just the CSC8502 texturedvertex shader
//uniform mat4 viewMatrix;
//uniform mat4 projMatrix; //If only using this shader for full screen quads, mvp unneccesary 
//uniform mat4 textureMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out Vertex {
    vec2 texCoord;
} OUT;

void main(void) {
     //mat4 mvp = projMatrix * viewMatrix * modelMatrix;
    // gl_Position = mvp * vec4(position, 1.0);
     gl_Position = vec4(position, 1); //1 is the w component here
   //  OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
     OUT.texCoord = texCoord; 
}