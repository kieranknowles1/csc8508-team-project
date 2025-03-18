#version 330 core

uniform sampler2D laserTex;
uniform vec2 texelSize;
uniform float blurScale;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColor;

void main() {
    vec4 sum = vec4(0.0);
    float weights[5] = float[](0.227, 0.194, 0.121, 0.054, 0.016);

    for (int i = -2; i <= 2; i++) {
        vec2 offset = vec2(0.0, i * texelSize.y * blurScale);
        sum += texture(laserTex, IN.texCoord + offset) * weights[abs(i)];
    }

    if (sum.a < 0.01) discard;
    fragColor = sum;
}
