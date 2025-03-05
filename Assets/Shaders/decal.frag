#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D decalTexture;
uniform float alphaFade;
uniform vec4 decalColor;

void main() {
    vec4 textureColor = texture(decalTexture, TexCoord);
    
    if (decalColor.a < 0.1) discard; // Discard transparent fragments

    // Modulate texture color with decal color
    vec4 finalColor = textureColor * decalColor;

    // Reduce alpha over time for fading effect
    finalColor.a *= alphaFade;

    // Output final color
    FragColor = finalColor;
}