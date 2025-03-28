#version 330 core 

uniform sampler2D diffuseTex; 

uniform int isVertical;

in Vertex {
   vec2 texCoord;
   } IN;

out vec4 fragColor;

const float scaleFactors[7] = 
      float[](0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006);
      
void main(void) {
     fragColor = vec4(0, 0, 0, 1);

     vec2 delta = vec2(0, 0);

     if(isVertical == 1) {
          delta = dFdy(IN.texCoord);
     }
     else {
          delta = dFdx(IN.texCoord);
     }
     for(int i = 0; i < 7; i++) {
        vec2 offset = delta * (i-3);
        vec4 tmp = texture(diffuseTex, IN.texCoord.xy + offset); 
        fragColor += tmp * scaleFactors[i];
        
     }
  
}

