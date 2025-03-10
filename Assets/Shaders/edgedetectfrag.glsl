#version 330 core

uniform sampler2D sceneTex;

uniform vec2 windowSize;

in Vertex {
    vec2 texCoord;
    } IN;

out vec4 fragColor;

//could maybe use windowsize like in vignettefrag for offsets instead
/*const*/ float offset_x = 1/windowSize.x; //do these have to be const floats?
/*const*/ float offset_y = 1/windowSize.y; //(number of pixels in y axis)

float kernel[9] = float[] ( //defines weightings of each pixel. Should add up to 1. If less than 1, scene darkens and if greater than 1 it brightens.
      1,  1, 1,             //here, they add up to zero to darken everything besides the edges which will be highlighted
      1, -8, 1,
      1,  1, 1
);

//other kernels to try:  0, -1,  0      1/9, 1/9, 1/9,    -2, -1, 0,
//                      -1,  5, -1      1/9, 1/9, 1/9,    -1,  1, 1,
//                       0, -1,  0      1/9, 1/9, 1/9      0,  1, 2
vec2 offsets[9] = vec2[] ( 
      vec2(-offset_x, offset_y),  vec2(0.0f, offset_y),  vec2(offset_x, offset_y), 
      vec2(-offset_x, 0.0f),      vec2(0.0f, 0.0f),      vec2(offset_x, 0.0f),
      vec2(-offset_x, -offset_y), vec2(0.0f, -offset_y), vec2(offset_x, -offset_y)
      ); 

void main(void) {
     vec3 colour = vec3(0.0f);
     for (int i = 0; i < 9; i++) //idk why no brackets here
         colour += vec3(texture(sceneTex, IN.texCoord.st+offsets[i]))* kernel[i];
     fragColor = vec4(colour, 1.0f);
     }