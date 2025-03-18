struct EdgeDetectMatrices {
    mat4 invProj;
    mat4 invView;
};

vec3 DepthToWorldPosition(vec2 texCoords, double depth, EdgeDetectMatrices mats) {
    // Convert texture coordinates to (-1 to 1 range)
    vec4 ndc = vec4(texCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    // Transform to view space
    vec4 viewPos = mats.invProj * ndc;
    viewPos /= viewPos.w;
    // Transform to world space
    vec4 worldPos = mats.invView * viewPos;
    return worldPos.xyz;
}


vec3 calculateNormal(vec2 texCoord, EdgeDetectMatrices mats, vec2 invWinSize){
    //sample centre and offsets
    double depthCenter = texture(depthTex, texCoord).r;
    double depthRight = texture(depthTex, texCoord + vec2(invWinSize.x, 0)).r;
    double depthUp = texture(depthTex, texCoord + vec2(0, invWinSize.y)).r;

    //convert to world space
    vec3 P = DepthToWorldPosition(texCoord, depthCenter, mats);
    vec3 Pr = DepthToWorldPosition(texCoord + vec2(invWinSize.x, 0), depthRight, mats);
    vec3 Pu = DepthToWorldPosition(texCoord + vec2(0, invWinSize.y), depthUp, mats);

    // Compute normal using cross product of tangent vectors
    vec3 normalWorld = normalize(cross(Pr - P, Pu - P));
    return normalWorld;
}



vec2 offsets[25] = vec2[] (
    vec2(-2.0,  2.0), vec2(-1.0,  2.0), vec2(0.0,  2.0), vec2(1.0,  2.0), vec2(2.0,  2.0),
    vec2(-2.0,  1.0),      vec2(-1.0,  1.0),      vec2(0.0,  1.0),      vec2(1.0,  1.0),      vec2(2.0,  1.0),
    vec2(-2.0,  0.0),           vec2(-1.0,  0.0),           vec2(0.0,  0.0),           vec2(1.0,  0.0),           vec2(2.0,  0.0),
    vec2(-2.0, -1.0),      vec2(-1.0, -1.0),      vec2(0.0, -1.0),      vec2(1.0, -1.0),      vec2(2.0, -1.0),
    vec2(-2.0, -2.0), vec2(-1.0, -2.0), vec2(0.0, -2.0), vec2(1.0, -2.0), vec2(2.0, -2.0)
);



vec4 doEdgeDetect(vec4 colour, vec2 texCoord, vec2 invWinSize, EdgeDetectMatrices mats) {
   vec3 normalWorld = calculateNormal(texCoord, mats, invWinSize);
   for (int i = 0; i < 25; i++) {
        vec2 offset = offsets[i] * invWinSize;
       vec3 normalWorld2 =calculateNormal(texCoord+offset, mats, invWinSize); // check surrounding normals
       if(distance(normalWorld,normalWorld2) >= 0.8f){ // normals are substantially different
           colour = vec4(0,0,0,1);
           break;
        }
    }
   return  colour;
 }
