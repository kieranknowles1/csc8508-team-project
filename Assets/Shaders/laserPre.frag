#version 400 core

uniform sampler2D laserTex;
uniform sampler2D oldLaserTex;
uniform float dt;

in Vertex {
    vec2 correctedTexCoord;
    vec2 originalTexCoord;
} IN;

out vec4 fragColor;

void main() {
    vec4 oldLaser = texture(oldLaserTex, IN.correctedTexCoord);

    // Fade out the previous frame's laser
    oldLaser.a *= 0.925;
    if (oldLaser.a <= 0.01) {
        oldLaser.a = 0;
    }

    vec4 newLaser = texture(laserTex, IN.originalTexCoord);
    fragColor = max(oldLaser, newLaser);
}
