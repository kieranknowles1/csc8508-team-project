#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;
uniform float time;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;



void main(void) {
    vec3 forward = normalize(endPosition - startPosition);

    // Pependicular vectors
    vec3 perp1 = normalize(abs(forward.x) > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0));
    vec3 perp2 = normalize(cross(forward, perp1));
    perp1 = normalize(cross(perp2, forward));

    // Length along ray
    float laserLength = length(endPosition - startPosition);
    float zMapped = (position.z+10.0f) * 0.05;
    float midPointDist = abs(zMapped - 0.5);
    float waveDistortionFactor = smoothstep(0.5, 0.25, midPointDist);

    // Spirals depends on length, each spiral is wavelength long;
    const float wavelength = 60.0;
    float numSpirals = laserLength / wavelength;

    float waveAngle = (-time * 10.0) + (zMapped * numSpirals * 6.283185); 
    float waveAngle2 = (-time * 20.0) + (zMapped * numSpirals * 6.283185); 

    // Decrese effect at start/end
    float waveRadius = 0.1 * thickness * waveDistortionFactor;
   
    vec3 waveOffset = (((perp1 * (cos(waveAngle)+(sin(waveAngle2)/4.0f))) + 
                      (perp2 * (sin(waveAngle))+(cos(waveAngle2)/4.0f)))
                      * waveRadius) * (50.0*(sqrt(numSpirals)));

    // Apply distortions and make thicker in centre
    vec3 worldPos = startPosition 
                  + forward * (zMapped * laserLength) 
                  + perp1 * (position.x * (thickness*(clamp(0,1,waveDistortionFactor+0.6f))) * 0.5) 
                  + perp2 * (position.y * (thickness*(clamp(0,1,waveDistortionFactor+0.6f))) * 0.5)    
                  + waveOffset;


    gl_Position = projMatrix * viewMatrix * vec4(worldPos, 1.0);
}
