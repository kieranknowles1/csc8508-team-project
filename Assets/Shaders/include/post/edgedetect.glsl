struct EdgeDetectMatrices {
    mat4 invProj;
    mat4 invView;
};

vec3 DepthToWorldPosition(vec2 texCoords, double depth, EdgeDetectMatrices mats) {
    // Convert texture coordinates to (-1 to 1 range)
    vec4 ndc = vec4(texCoords * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    // Transform to view space
    vec4 viewPos = mul(mats.invProj, ndc);
    viewPos /= viewPos.w;
    // Transform to world space
    vec4 worldPos = mul(mats.invView, viewPos);
    return worldPos.xyz;
}


vec3 calculateNormal(vec2 texCoord, EdgeDetectMatrices mats, vec2 invWinSize){
    //sample centre and offsets
    double depthCenter = depthSample(texCoord);
    double depthRight = depthSample(texCoord + vec2(invWinSize.x, 0));
    double depthUp = depthSample(texCoord + vec2(0, invWinSize.y));

    //convert to world space
    vec3 P = DepthToWorldPosition(texCoord, depthCenter, mats);
    vec3 Pr = DepthToWorldPosition(texCoord + vec2(invWinSize.x, 0), depthRight, mats);
    vec3 Pu = DepthToWorldPosition(texCoord + vec2(0, invWinSize.y), depthUp, mats);

    // Compute normal using cross product of tangent vectors
    vec3 normalWorld = normalize(cross(Pr - P, Pu - P));
    return normalWorld;
}
