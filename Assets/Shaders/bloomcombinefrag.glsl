#version 330 core

uniform sampler2D sceneTex; //This shader combines the blurred lights with the scene as is to create the bloom effect
uniform sampler2D blurredLights;

uniform bool bloomOn;

in Vertex{
   vec2 texCoord;
   }IN;

out vec4 fragColor;

void main(void) {
    const float gamma = 2.2;
    vec3 hdrColour = texture(sceneTex, IN.texCoord).rgb;  
    vec3 bloomColour = texture(blurredLights, IN.texCoord).rgb;
    if (bloomOn == true) {
       hdrColour += bloomColour;
    } 
   
    fragColor = vec4(hdrColour, 1.0);
}