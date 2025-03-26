#version 330 core

uniform sampler2D diffuseTex; //will take in the scene, and output the scene but also another texture that only includes the bright parts of the scene

in Vertex {
   vec2 texCoord;
   } IN;

out vec4 fragColour[2];

void main(void) {

    vec3 texColour = texture(diffuseTex, IN.texCoord).rgb;

    //quantify brightness:
    float brightness = dot(texColour.rgb, vec3(0.2126, 0.7152, 0.0722)); //values from LearnOpenGL. Brightness is found by converting to greyscale

    //output the scene as is:
    fragColour[0] = vec4(texColour, 1.0);

    //render parts of scene above a threshold brightness into another seperate texture:
    if (brightness > 0.1f) { //can be higher than one because we are using HDR tonemapping and floating point textures SHOULD BE 1.0 however don't see much unless set to 0.1 or less
         fragColour[1] = vec4(texColour.rgb, 1.0);
         }
    else {
         fragColour[1] = vec4(0.0, 0.0, 0.0, 1.0); //if not above threshold brightness, output black
         }
}