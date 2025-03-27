#version 400 core

uniform vec4 inColour;
uniform sampler2D depthTex;
uniform vec2 windowSize;

in float depth;

out vec4 fragColor;

void main() {
    vec2 screenUV = gl_FragCoord.xy / windowSize;
    float texDepth = texture(depthTex, screenUV).r; 
    if (depth > texDepth) {
        discard;
    }
    vec4 strongerColour = inColour * 5.0f;
    strongerColour.a = 1.0f;
    fragColor = strongerColour;
}
