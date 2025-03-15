#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;
uniform float time;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out Vertex {
    vec2 TexCoord;
    float waveDist;
} OUT;

void main(void) {
    vec3 forward = normalize(endPosition - startPosition);

    // Choose a stable perpendicular vector
    vec3 perp1 = normalize(abs(forward.x) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0));
    
    // Generate the second perpendicular vector
    vec3 perp2 = normalize(cross(forward, perp1));
    
    // Recompute perp1 to ensure it's perfectly orthogonal
    perp1 = normalize(cross(perp2, forward));


    float laserLength = length(endPosition - startPosition);

    float zMapped = (position.z+10.0f) * 0.05;
    
    float midPointDist = abs(zMapped - 0.5);

    float waveDistortionFactor = smoothstep(0.5, 0.25, midPointDist);

    // Keep a constant wavelength for a smooth curve
    const float wavelength = 60.0;  // Adjust this for spacing
    float numSpirals = laserLength / wavelength;

    // Use a proper circular oscillation for smooth rotation
    float waveAngle = (-time * 10.0) + (zMapped * numSpirals * 6.283185); // Consistent speed + adaptive spirals

    float waveRadius = 0.1 * thickness * waveDistortionFactor;
   
    vec3 waveOffset = ((perp1 * cos(waveAngle) + perp2 * sin(waveAngle)) * waveRadius) * (50.0*(sqrt(numSpirals)));

    // Apply distortions while maintaining circular motion
    vec3 worldPos = startPosition 
                  + forward * (zMapped * laserLength) 
                  + perp1 * (position.x * (thickness*(clamp(0,1,waveDistortionFactor+0.5f))) * 0.5) 
                  + perp2 * (position.y * (thickness*(clamp(0,1,waveDistortionFactor+0.5f))) * 0.5)    
                  + waveOffset; // Circular oscillation

    OUT.waveDist = waveDistortionFactor;
    OUT.TexCoord = texCoord;

    gl_Position = projMatrix * viewMatrix * vec4(worldPos, 1.0);
}
