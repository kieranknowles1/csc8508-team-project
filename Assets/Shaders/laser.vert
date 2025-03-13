#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;

layout(location = 0) in vec3 position; // Object-space sphere
layout(location = 2) in vec2 texCoord;

out vec2 TexCoord;

void main(void) {
    vec3 forward = normalize(endPosition - startPosition);

    // Create perpendicular basis vectors
    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(forward, up));

    // Ensure up is truly perpendicular (recompute if necessary)
    up = normalize(cross(right, forward));

    float laserLength = length(endPosition - startPosition);

    // Shift position.z from [-1,1] ? [0,1] so it starts at startPosition
    float zMapped = (position.z + 1.0) * 0.5;  

    // Transform sphere into a stretched cylinder along the laser direction
    vec3 worldPos = startPosition 
                  + forward * (zMapped * laserLength)  // Starts at startPosition now
                  + right * (position.x * thickness * 0.5)  // Expand radially in X
                  + up * (position.y * thickness * 0.5);   // Expand radially in Y

    gl_Position = projMatrix * viewMatrix * vec4(worldPos, 1.0);
    TexCoord = texCoord;
}
