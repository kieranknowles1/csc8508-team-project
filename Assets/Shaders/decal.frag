#version 330 core

in vec4 FragPos;
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D decalTexture;
uniform sampler2D depthTexture;   // The main scene depth
uniform float alphaFade;
uniform vec4 decalColor;

// uniform float nearPlane;
// uniform float farPlane;
const float nearPlane = 0.1;
const float farPlane = 100.0;

uniform int screenWidth;
uniform int screenHeight;

// Function to convert non-linear depth values to linear depth values
// https://learnopengl.com/Advanced-OpenGL/Depth-testing
float LinearizeDepth(float depth) {
    return (2.0 * nearPlane) / (farPlane + nearPlane - depth * (farPlane - nearPlane));
}

void main() {
    vec4 textureColor = texture(decalTexture, TexCoord);

    
    // Get scene depth value from the depth texture
    vec2 sceneTexCoords = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    float sceneDepthNonLinear = texture(depthTexture, sceneTexCoords).r;
    // float sceneDepthNonLinear = texture(depthTexture, FragPos.xy).r;

    // Get the depth value of the decal fragment from the decal depth texture
    float decalFragDepthNonLinear = gl_FragCoord.z;

    // Convert non-linear depth values to linear depth values
    float sceneFragDepth = LinearizeDepth(sceneDepthNonLinear);
    float decalFragDepth = LinearizeDepth(decalFragDepthNonLinear);

    if (abs(decalFragDepth - sceneFragDepth) > 0.001) {
        // decalColor.a = 0.0; // Fully transparent
        discard;
    }
    
    if (decalColor.a < 0.1) discard; // Discard transparent fragments

    // Modulate texture color with decal color
    vec4 finalColor = textureColor * decalColor;

    // Reduce alpha over time for fading effect
    finalColor.a *= alphaFade;

    // Output final color
    FragColor = finalColor;
}