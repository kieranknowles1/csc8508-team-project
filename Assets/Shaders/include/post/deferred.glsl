vec3 doDeferredLight(
    vec3 scene,
    vec3 lightDiffuse,
    vec3 lightSpecular,
    float ambient
) {
    vec3 result;
    result.xyz  = scene * ambient; //ambient
    result.xyz += scene * lightDiffuse; //lambert
    result.xyz += lightSpecular;
    return result;
}
