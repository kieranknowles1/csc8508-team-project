#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;
uniform float time;

out float depth;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

void main(void) {
    vec3 forward = normalize(endPosition - startPosition);

    // Perpendicular vectors
    vec3 perp1 = normalize(abs(forward.x) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0));
    vec3 perp2 = normalize(cross(forward, perp1));
    perp1 = normalize(cross(perp2, forward));

    // Length along the laser
    float laserLength = length(endPosition - startPosition);
    float zMapped = (position.z + 10.0) * 0.05;
    float midPointDist = abs(zMapped - 0.5);
    float waveDistortionFactor = smoothstep(0.5, 0.25, midPointDist);

    // Main wave (spiral effect)
    const float wavelength = 60.0;
    float numSpirals = laserLength / wavelength;
    float waveAngle = (-time * 10.0) + (zMapped * numSpirals * 6.283185); 
    float waveAngle2 = (-time * 5.0) + (zMapped * numSpirals * 6.283185); 
    float waveRadius = 0.1 * thickness * waveDistortionFactor;
    vec3 baseWaveOffset = (
        ((perp1 * (cos(waveAngle)/1.0f)) + (perp1 * ((cos(waveAngle2) / 2.0)+1.0f))) + 
        (perp2 * (sin(waveAngle)/1.0f)) + (perp2 * ((sin(waveAngle2) / 2.0)+1.0f))
    ) * waveRadius * (25.0 * sqrt(numSpirals));

    // Additional wobble effects
    float rippleFreq = 0.15 * numSpirals;
    float rippleSpeed = time * -50.0;
    float rippleAngle1 = zMapped * rippleFreq * 6.283185 + rippleSpeed;
    float rippleAngle2 = zMapped * rippleFreq * 8.283185 + rippleSpeed * 0.5;
    float rippleStrength = 0.03 * waveDistortionFactor * sqrt(laserLength);  // Small contribution
    vec3 rippleOffset = (
        (perp1 * sin(rippleAngle1)) + (perp2 * cos(rippleAngle2))
    ) * rippleStrength;

    // Apply distortions and make thicker in the center
    vec3 worldPos = startPosition 
                  + forward * (zMapped * laserLength) 
                  + perp1 * (position.x * (thickness * clamp(0, 1, waveDistortionFactor + 0.6))) * 0.5 
                  + perp2 * (position.y * (thickness * clamp(0, 1, waveDistortionFactor + 0.6))) * 0.5    
                  + baseWaveOffset
                  + rippleOffset;


    vec4 clipSpacePos = projMatrix * viewMatrix * vec4(worldPos, 1.0);
    gl_Position = clipSpacePos;
    depth = (clipSpacePos.z / clipSpacePos.w) * 0.5 + 0.5; //world space position

}
