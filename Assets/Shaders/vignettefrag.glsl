#version 330 core

uniform sampler2D diffuseTex;
//uniform vec2 windowSize; ///Currently, windowSize.x is zero as is windowSize.y
uniform float windowSizex;
uniform float windowSizey;
uniform vec3 effectColour;
uniform float intensity = 1.0f;
uniform float time;

in Vertex {
   vec2 texCoord;
   }IN;

   out vec4 fragColor;

   void main(void) { 
     vec3 texColour = texture(diffuseTex, IN.texCoord).rgb; //sample the framebuffer texture 
     fragColor.rgb = texColour;
    //add vignette:
    //need to change fragment colour (brightness and colour) based on distance from screen centre
    vec2 resolution = vec2(windowSizex, windowSizey); //number of pixels in each axis
    //vec2 centreposition = resolution * 0.5f;
    vec2 centreposition = vec2(0.5, 0.5); //NDC screen centre
    float dist = length((gl_FragCoord.xy / resolution) - centreposition); //distance from centre. Make sure the fragment coordinate is converted to ndc first
    //fragColor.rgb *= 1-dist*0.001; //NOTE: dist is less than 1 but converges to 1
    //fragColor.rgb *= (1-pow(dist, 0.5));
    //fragColor.rgb = mix(vec3(0.5, 0.0, 0.0), fragColor.rgb, 1-(dist*dist) ); //last parameter is a weighting. Middle of screen looks kinda blue shifted (1-dist)*2.0
    float intensefactor = intensity;
    intensefactor *= 1+(0.05*sin(time*1.5));
   // fragColor.rgb = mix(fragColor.rgb, effectColour, pow(dist, invRange)*intensity); //dist*dist*2 //pow(dist, 2) //pow(dist, invRange)*intensity
   fragColor.rgb = mix(effectColour, fragColor.rgb, clamp(1-(pow(dist, 2.0)*intensefactor), 0, 1)); //last parameter is a weighting, higher weighting = lower influence of vignette
   //we want to make sure the weighting is clamped between 0 and 1.
   // fragColor.rgb = texColour; 

     fragColor.a = 1.0f;

   }