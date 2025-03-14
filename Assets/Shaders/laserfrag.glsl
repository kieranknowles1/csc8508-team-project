#version 400 core

uniform uint inColour;
in Vertex
{
   vec2 TexCoord;
   float waveDist;
} IN;

out vec4 FragColor;

vec4 getColour(uint col){

// Extract color components from the uint
    float alpha =1.0;
    float blue = clamp(float((col >> 8) & 0xFF) / 255.0, 0, 1);
    float green = clamp(float((col >> 16) & 0xFF) / 255.0, 0, 1);
    float red = clamp(float((col >> 24) & 0xFF) / 255.0, 0, 1);

    // Create vec4 color
    return vec4(red, green, blue, alpha);

    }

void main() {
    // Use smoothstep to fade out the color along the laser length
    vec3 laserColor = vec3(1.0, 0.0, 0.0);  // Pure red

    // Modify the laser color based on waveDist to create a smooth fade along the length
    laserColor *= IN.waveDist;  // Fade from start to end based on the smoothstep transition

    FragColor = getColour(inColour);  // Set the color of the fragment
}
