#version 330 core


uniform sampler2D sceneTex;
uniform sampler2D laserTex;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
    vec4 colour  = texture(sceneTex, IN.texCoord);
    vec4 laser =  texture(laserTex, IN.texCoord);
    if(laser.a >= 0.01f){
        colour = laser;
    }
    colour.a    = 1.0;
    fragColour =colour;
}
