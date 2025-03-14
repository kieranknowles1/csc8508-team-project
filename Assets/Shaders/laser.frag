#version 400 core

uniform vec4 inColour;
in Vertex
{
   vec2 TexCoord;
   float waveDist;
} IN;

out vec4 FragColor;


void main() {
    // Use smoothstep to fade out the color along the laser length
    vec3 laserColor = vec3(1.0, 0.0, 0.0);  // Pure red

    // Modify the laser color based on waveDist to create a smooth fade along the length
    laserColor *= IN.waveDist;  // Fade from start to end based on the smoothstep transition

    FragColor = inColour;  // Set the color of the fragment
}
