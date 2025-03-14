#version 330 core

in Vertex {
    vec2 texCoord;
} IN;

out vec4 FragColor;

uniform sampler2D sceneTexture;   // The main scene texture
uniform sampler2D decalTexture;   // The decal buffer texture


void main() {
    vec4 sceneColor = texture(sceneTexture, IN.texCoord);
    vec4 decalColor = texture(decalTexture, IN.texCoord);


    // Blend decals over the scene
    vec3 finalColor = mix(sceneColor.rgb, decalColor.rgb, decalColor.a);
    float finalAlpha = sceneColor.a; // Keep original alpha of the scene

    FragColor = vec4(finalColor, finalAlpha);
}