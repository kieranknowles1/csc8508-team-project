#version 330 core

uniform sampler2D diffuseTex;
//uniform vec2 windowSize; ///Currently, windowSize.x is zero as is windowSize.y
uniform float windowSizex;
uniform float windowSizey;

in Vertex {
   vec2 texCoord;
   }IN;

   out vec4 fragColor;

   void main(void) { 
     vec3 texColour = texture(diffuseTex, IN.texCoord).rgb; //sample the framebuffer texture 
     fragColor.rgb = texColour;
    //add vignette:
    //need to change fragment colour (brightness and colour) based on distance from screen centre
    vec2 resolution = vec2(windowSizex, windowSizey);
    //vec2 centreposition = resolution * 0.5f;
    vec2 centreposition = vec2(0.5, 0.5);
    float dist = length((gl_FragCoord.xy / resolution) - centreposition); //distance from centre. Small dist means close to centreposition
    //fragColor.rgb *= 1-dist*0.001;
    //fragColor.rgb *= 1-dist*1.4;
    fragColor.rgb = mix(vec3(1.0, 0.0, 0.0), fragColor.rgb, 1 - dist*0.8); 
   // fragColor.rgb = texColour;

     fragColor.a = 1.0f;

   }