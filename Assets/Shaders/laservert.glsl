#version 400 core

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec3 startPosition;
uniform vec3 endPosition;
uniform float thickness;
uniform float time;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out Vertex
{
    vec2 TexCoord;
    float waveDist;
} OUT;

void main(void) {
    vec3 forward = normalize(endPosition - startPosition);

    vec3 up = vec3(0, 1, 0);
    vec3 right = normalize(cross(forward, up));

    up = normalize(cross(right, forward));

    float laserLength = length(endPosition - startPosition);

    float zMapped = (position.z + 1.0) * 0.5;
    float midPointDist = abs(zMapped - 0.5);

    float waveDistortionFactor = smoothstep(0.5, 0.25, midPointDist);

    float sineWaveDistortion = sin(time * 3.0 + zMapped * 10.0*(laserLength *0.001f)) * 0.1 * thickness * waveDistortionFactor;

    float cosWaveDistortion = cos(time * 3.0 + zMapped * 10.0*(laserLength *0.001f)) * 0.1 * thickness * waveDistortionFactor;

    sineWaveDistortion *= (laserLength *0.0005f);
    
    cosWaveDistortion *= (laserLength *0.0005f);

    vec3 worldPos = startPosition 
                  + forward * (zMapped * laserLength) 
                  + right * (position.x * thickness * 0.5) 
                  + up * (position.y * thickness * 0.5)    
                  + vec3( (cosWaveDistortion * 35.0), (sineWaveDistortion * 35.0),0); 


    OUT.waveDist = waveDistortionFactor;
    OUT.TexCoord = texCoord;

    gl_Position = projMatrix * viewMatrix * vec4(worldPos, 1.0);
}
