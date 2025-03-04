#version 330 core

in Vertex {
    vec2 texCoord;
} IN;

out vec4 FragColor;

uniform sampler2D sceneTexture;   // The main scene texture
uniform sampler2D decalTexture;   // The decal buffer texture
uniform sampler2D depthTexture;   // The SCENE DEPTH buffer texture
uniform sampler2D decalDepthTexture; // The DECAL DEPTH buffer texture

void main() {
    vec4 sceneColor = texture(sceneTexture, IN.texCoord);
    vec4 decalColor = texture(decalTexture, IN.texCoord);

    // Get scene depth value from the depth texture
    float sceneDepth = texture(depthTexture, IN.texCoord).r;

    // Get the depth value of the decal fragment from the decal depth texture
    float decalFragDepth = texture(decalDepthTexture, IN.texCoord).r;

    // if the decal is in front of the scene, discard the fragment
    // This prevents the floating decals issue when the decal is in front of the scene
    if (decalFragDepth + 0.001 < sceneDepth) {
        decalColor.a = 0.0; // Fully transparent
    }

    // Blend decals over the scene
    vec3 finalColor = mix(sceneColor.rgb, decalColor.rgb, decalColor.a);
    float finalAlpha = sceneColor.a; // Keep original alpha of the scene

    FragColor = vec4(finalColor, finalAlpha);

    //FragColor.a = 1.0f;
}