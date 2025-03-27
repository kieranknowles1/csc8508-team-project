#version 330 core

#include "include/post/deferred.glsl"

uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;
uniform sampler2D glossBufferTex;
uniform sampler2D specularBufferTex;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
    vec3 diffuse  = texture(diffuseTex, IN.texCoord).xyz;
    vec3 light    = texture(diffuseLight, IN.texCoord).xyz;
    vec3 specular = texture(specularLight, IN.texCoord).xyz;

    float gloss = texture(glossBufferTex, IN.texCoord).r; // extracting glossiness from red channel
    float specularStrength = texture(specularBufferTex, IN.texCoord).r; // extracting specular intensity

    // Clamp values to prevent invalid output
    gloss = max(gloss, 0.05); // Avoid zero gloss
    specularStrength = max(specularStrength, 0.05); // Avoid zero specular

    // TODO: Implement specular and glossiness
    // fragColour.rgb = pow(fragColour.rgb, vec3(1.0 / 2.2f)); //gamma correction, maybe should be in another shader. Probably should be done last
    vec4 colour = vec4(0,0,0,0);
    colour.rgb = doDeferredLight(
       diffuse,
       light,
       specular * specularStrength,
       0.05
    );
    colour.a    = 1.0;

    fragColour =colour;
}
