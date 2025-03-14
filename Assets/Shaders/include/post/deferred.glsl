vec3 doDeferredLight(
    vec3 scene,
    vec3 lightDiffuse,
    vec3 lightSpecular
) {
    vec3 result;
    result.xyz  = scene * 0.05; //ambient
    result.xyz += scene * lightDiffuse; //lambert
    result.xyz += lightSpecular;
    return result;
}
