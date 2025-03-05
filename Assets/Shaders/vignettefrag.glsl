#version 330 core

uniform sampler2D diffuseTex;
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
     vec2 centreposition = vec2(0.5, 0.5); //NDC screen centre
     float dist = length((gl_FragCoord.xy / resolution) - centreposition); //distance from centre. Make sure the fragment coordinate is converted to ndc first
     float intensefactor = intensity;
     intensefactor *= 1+(0.05*sin(time*1.5)); //intensity pulsates in time
     fragColor.rgb = mix(effectColour, fragColor.rgb, clamp(1-(pow(dist, 2.0)*intensefactor), 0, 1)); //last parameter is a weighting, higher weighting = lower influence of vignette
     //we want to make sure the weighting is clamped between 0 and 1.

     fragColor.a = 1.0f;

   }