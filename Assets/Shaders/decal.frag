#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D decalTexture;
uniform float alphaFade;

void main() {
    vec4 decalColor = texture(decalTexture, TexCoord);
    if (decalColor.a < 0.1) discard; // Discard transparent fragments

    // Apply gamma correction as the decal looks washed out (coz of linear-space lighting calculations)
    // This is a simple approximation of the sRGB color space (which is the default color space for most monitors)
    decalColor.rgb = pow(decalColor.rgb, vec3(2.2));

    // Reduce alpha over time for fading effect
    decalColor.a *= alphaFade;

    // Output final color
    FragColor = decalColor;
}