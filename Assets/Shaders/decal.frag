#version 330 core

in vec3 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D decalTexture;
uniform float alphaFade;

void main() {
    vec4 decalColor = texture(decalTexture, TexCoord);
    //if (decalColor.a < 0.1) discard; // Discard transparent fragments

    // Reduce alpha over time for fading effect
    decalColor.a = max(decalColor.a - alphaFade, 0.0);

    // Output final color
    //FragColor = decalColor;
    
    // Debug output (red = missing texture, green = valid UVs)
    if (decalColor.rgb == vec3(0.0)) {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red if texture fails
    } else {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Green if UVs work
    }
}