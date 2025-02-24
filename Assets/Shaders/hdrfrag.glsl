#version 330 core

uniform sampler2D hdrTex;
//uniform vec2 windowSize; ///Currently, windowSize.x is zero as is windowSize.y
uniform float windowSizex;
uniform float windowSizey;

in Vertex {
   vec2 texCoord;
   }IN;

   out vec4 fragColor;

   void main(void) { 
     vec3 texColour = texture(hdrTex, IN.texCoord).rgb; //sample the framebuffer texture 

     //apply Reinhardt tone mapping to convert HDR back to LDR: L/L+1
     vec3 tonemapColour = texColour / (texColour + vec3(1.0f)); 

    //gamma correction not needed since scenefrag already gamma corrects when rendering the scene initially. Could have all gamma correction done here instead though
     fragColor.rgb = tonemapColour;

    //add vignette:
    //need to change fragment colour (brightness and colour) based on distance from screen centre
    vec2 resolution = vec2(windowSizex, windowSizey);
    vec2 centreposition = resolution * 0.5f;
    float dist = length(gl_FragCoord.xy - centreposition); //distance from centre. Small dist means close to centreposition
    fragColor.rgb *= 1-dist*0.001;

     fragColor.a = 1.0f;

   }