#version 330 core//should this be 400?

uniform sampler2D hdrTex;

in vertex {
   vec2 texCoord;
   }IN;

   out vec4 fragColor;

   void main(void) { 
     vec3 texColour = texture(hdrTex, IN.texCoord).rgb; //sample the framebuffer texture 

     //apply Reinhardt tone mapping to convert HDR back tp LDR: L/L+1
    // vec3 tonemapColour = texColour / (texColour + vec3(1.0f)); 
     //apply gamma correction (by raising to power of 1/2.2):
     //vec3 gammacorrectColour = pow(tonemapColour, vec3(1.0f/2.2f));

     //fragColor.rgb = gammacorrectColour; //gamma correction not needed since scenefrag already gamma corrects when rendering the scene initially
   //  fragColor.rgb = tonemapColour; //tonemapColour 
     fragColor.rgb = texColour;
   
     fragColor.a = 1.0f;
   }