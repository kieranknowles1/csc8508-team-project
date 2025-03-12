#version 330 core

uniform sampler2D sceneTex;
uniform sampler2D depthTex;

uniform vec2 windowSize;
uniform float nearPlane;
uniform float farPlane;

uniform mat4 inverseProjMatrix;
uniform mat4 inverseViewMatrix;


in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColor;

//could maybe use windowsize like in vignettefrag for offsets instead
float offset_x = 1/windowSize.x; //do these have to be const floats?
float offset_y = 1/windowSize.y; //(number of pixels in y axis)


vec3 DepthToWorldPosition(vec2 texCoords, float depth) {
    // Convert texture coordinates to (-1 to 1 range)
    vec4 ndc = vec4(texCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    // Transform to view space
    vec4 viewPos = inverseProjMatrix * ndc;
    viewPos /= viewPos.w;
    // Transform to world space
    vec4 worldPos = inverseViewMatrix * viewPos;
    return worldPos.xyz;
}


vec3 calculateNormal(vec2 texCoord){
    //sample centre and offsets
    float depthCenter = texture(depthTex, texCoord).r;
    float depthRight = texture(depthTex, texCoord + vec2(offset_x, 0)).r;
    float depthUp = texture(depthTex, texCoord + vec2(0, offset_y)).r;

    //convert to world space
    vec3 P = DepthToWorldPosition(texCoord, depthCenter);
    vec3 Pr = DepthToWorldPosition(texCoord + vec2(offset_x, 0), depthRight);
    vec3 Pu = DepthToWorldPosition(texCoord + vec2(0, offset_y), depthUp);

    // Compute normal using cross product of tangent vectors
    vec3 normalWorld = normalize(cross(Pr - P, Pu - P));
    return normalWorld;
}



vec2 offsets[25] = vec2[] (
    vec2(-2.0 * offset_x,  2.0 * offset_y), vec2(-offset_x,  2.0 * offset_y), vec2(0.0,  2.0 * offset_y), vec2(offset_x,  2.0 * offset_y), vec2(2.0 * offset_x,  2.0 * offset_y),
    vec2(-2.0 * offset_x,  offset_y),      vec2(-offset_x,  offset_y),      vec2(0.0,  offset_y),      vec2(offset_x,  offset_y),      vec2(2.0 * offset_x,  offset_y),
    vec2(-2.0 * offset_x,  0.0),           vec2(-offset_x,  0.0),           vec2(0.0,  0.0),           vec2(offset_x,  0.0),           vec2(2.0 * offset_x,  0.0),
    vec2(-2.0 * offset_x, -offset_y),      vec2(-offset_x, -offset_y),      vec2(0.0, -offset_y),      vec2(offset_x, -offset_y),      vec2(2.0 * offset_x, -offset_y),
    vec2(-2.0 * offset_x, -2.0 * offset_y), vec2(-offset_x, -2.0 * offset_y), vec2(0.0, -2.0 * offset_y), vec2(offset_x, -2.0 * offset_y), vec2(2.0 * offset_x, -2.0 * offset_y)
);



void main(void) {
   vec4 colour = texture(sceneTex, IN.texCoord); //initially just render scene as is ////////
   vec3 normalWorld = calculateNormal(IN.texCoord);
   for (int i = 0; i < 25; i++) {
       vec3 normalWorld2 =calculateNormal(IN.texCoord+offsets[i]); // check surrounding normals
       if(distance(normalWorld,normalWorld2) > 0.9f){ // normals are substantially different
           colour = vec4(0,0,0,1);
           break;
        }
    }
   fragColor = colour;

 }
