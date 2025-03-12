#version 330 core

uniform sampler2D sceneTex;
uniform sampler2D depthTex;

uniform vec2 windowSize;

// uniform float nearPlane;  //This section is as in decal.frag
// uniform float farPlane;
const float nearPlane = 0.1;
const float farPlane = 100.0;

// Function to convert non-linear depth values to linear depth values
// https://learnopengl.com/Advanced-OpenGL/Depth-testing
float LinearizeDepth(float depth) {
    return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

//idea, sample depthTex, linearise these values then apply edge detection kernal to detect where edges exist in depth buffer. Then sample the colour texture and apply the 
//black outlines where there are edges in the depth buffer. Maybe use gl_FragCoord?


in Vertex {
    vec2 texCoord;
    } IN;

out vec4 fragColor;

//could maybe use windowsize like in vignettefrag for offsets instead
/*const*/ float offset_x = 1/windowSize.x; //do these have to be const floats?
/*const*/ float offset_y = 1/windowSize.y; //(number of pixels in y axis)

float kernel[9] = float[] ( //defines weightings of each pixel. Should add up to 1. If less than 1, scene darkens and if greater than 1 it brightens.
      -1,  -1,  -1,             //here, they add up to zero to darken everything besides the edges which will be highlighted
      -1,   9,  -1,             //BLACKBOARDISH effect
      -1,  -1,  -1
);

/*float kernel[9] = float[] (
       0, -1,  0,
      -1,  5, -1,
       0,  -1, 0
       );*/

/*float kernel[9] = float[] ( //this one looks too dark/doesn't render anything
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9,
      1/9, 1/9, 1/9
      );*/

/*float kernel[9] = float[] (
      -2, -1, 0, 
      -1,  1, 1,
       0,  1, 2
       );*/
/*
float kernel[9] = float[] (
      -1, 0, 1,
      -2, 1, 2, 
      -1, 0, 1
     );*/
//other kernels to try:  0, -1,  0      1/9, 1/9, 1/9,    -2, -1, 0,
//                      -1,  5, -1      1/9, 1/9, 1/9,    -1,  1, 1,
//                       0, -1,  0      1/9, 1/9, 1/9      0,  1, 2
vec2 offsets[9] = vec2[] ( 
      vec2(-offset_x, offset_y),  vec2(0.0f, offset_y),  vec2(offset_x, offset_y), //each value is the offset in pixels from the centre pixel
      vec2(-offset_x, 0.0f),      vec2(0.0f, 0.0f),      vec2(offset_x, 0.0f),
      vec2(-offset_x, -offset_y), vec2(0.0f, -offset_y), vec2(offset_x, -offset_y)
      ); 

void main(void) {//for edge detection kernels, the resulting value for a pixel is high at edges (rapid value changes)
     float SceneDepthNonLinear = texture(depthTex, IN.texCoord).r; //attempting to use the depth buffer to find edges instead of colour attachment
     float fragDepth = LinearizeDepth(SceneDepthNonLinear);

     vec3 colour = vec3(0.0f);
     vec3 tempCol = vec3(0.0f);
     colour = texture(sceneTex, IN.texCoord).rgb; //initially just render scene as is ////////
     for (int i = 0; i < 9; i++) {//idk why no brackets here
        /*if (vec3(texture(sceneTex, IN.texCoord.st+offsets[i])) * kernel[i]) > vec3(1.0f)) {
            colour = vec3(0, 0, 0);
            }*/
            tempCol += vec3(texture(depthTex, IN.texCoord.st+offsets[i])) * kernel[i]; //in theory this value should be large at edges only. This is the convolution.
     }
       if (length(tempCol) > 1) {//seemingly this needs to be exactly 1 to work with the current kernel 
                colour = vec3(0, 0, 0);
                }
         //colour += vec3(texture(depthTex, IN.texCoord.st+offsets[i]))* kernel[i]; //previously sampled the sceneTex
     fragColor = vec4(colour, 1.0f);
     }

     //using the offsets we can sample what would be the neighbouring pixel values seeing as these colour values are currently stored in a texture anyways