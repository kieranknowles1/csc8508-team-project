#version 330 core

//HDR Method from LearnOpenGL:
// https://learnopengl.com/Advanced-Lighting/HDR

uniform sampler2D hdrTex;
uniform bool hdrOn = true;

in Vertex {
   vec2 texCoord;
   }IN;

   out vec4 fragColor;

   void main(void) { 
     vec3 texColour = texture(hdrTex, IN.texCoord).rgb; //sample the framebuffer texture 
     if (!hdrOn) {
         fragColor.rgb = texColour;
         fragColor.a = 1;
         return;
         }

     //apply Reinhardt tone mapping to convert HDR back to LDR: L/L+1
     vec3 tonemapColour = texColour / (texColour + vec3(1.0f)); 

    //gamma correction not needed since scenefrag already gamma corrects when rendering the scene initially. Could have all gamma correction done here instead though
     fragColor.rgb = tonemapColour;
     fragColor.a = 1.0f;

   }