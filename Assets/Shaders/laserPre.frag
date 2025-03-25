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
    float multiplier = (50.0f * dt);
    vec4 oldLaser = texture(oldLaserTex, IN.correctedTexCoord);

    // Fade out the previous frame's laser
    oldLaser.a *= multiplier;
    if (oldLaser.a <= 0.01) {
        oldLaser = vec4(0,0,0,0);
    }

    vec4 newLaser = texture(laserTex, IN.originalTexCoord);
    if(newLaser.a > oldLaser.a){
        oldLaser = newLaser;
    }

    fragColor = oldLaser;
}