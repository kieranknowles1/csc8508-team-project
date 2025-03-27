#version 330 core

uniform sampler2D sceneTex; //This shader combines the blurred lights with the scene as is to create the bloom effect
uniform sampler2D blurredLights;
//uniform float exposure;
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
    /* //Tone mapping, though not sure we should do that here if we have the hdr shader to do that later
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    //gamma correction, again not sure if we should do that here
    result = pow(result, vec3(1.0 / gamma));
    fragColor = vec4(result, 1.0); 
    */
    //Alternate ending:
    fragColor = vec4(hdrColour, 1.0);
}