struct LightOutput {
    vec4 diffuse;
    vec4 specular;
};

LightOutput doPointLight(
    LightState state,
    vec3 normal,
    vec3 cameraPos,
    vec2 texCoord,
    float depth,
    mat4 inverseProjView
) {
    vec3  ndcPos     = vec3(texCoord, depth) * 2.0 - 1.0;
    vec4  invClipPos = inverseProjView * vec4(ndcPos, 1.0);
    vec3  worldPos   = invClipPos.xyz / invClipPos.w;

    float dist       = length(state.position - worldPos);
    float atten      = 1.0 - clamp(dist / state.radius, 0.0, 1.0);

    if (atten <= 0.0) {
        LightOutput result;
        result.diffuse = vec4(0); result.specular = vec4(0);
        discard;
    }

    vec3 incident = normalize(state.position - worldPos);
    vec3 viewDir  = normalize(cameraPos - worldPos);
    vec3 halfDir  = normalize(incident + viewDir);

    float lambert    = clamp(dot(incident, normal), 0.0, 1.0);
    float rFactor    = clamp(dot(halfDir, normal), 0.0, 1.0);
    float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
    specFactor       = pow(specFactor, 80.0);
    vec3 attenuated  = state.color.xyz * atten;

    LightOutput result;
    result.diffuse = vec4(attenuated * lambert, 1.0) * state.intensity;
    result.specular = vec4(attenuated * specFactor * 0.33, 1.0) * state.intensity;
    return result;
}
