#version 330 core

#include "include/post/deferred.glsl"

uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;
uniform sampler2D lasers;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
    vec3 diffuse  = texture(diffuseTex, IN.texCoord).xyz;
    vec3 light    = texture(diffuseLight, IN.texCoord).xyz;
    vec3 specular = texture(specularLight, IN.texCoord).xyz;

    // fragColour.rgb = pow(fragColour.rgb, vec3(1.0 / 2.2f)); //gamma correction, maybe should be in another shader. Probably should be done last
    vec4 colour = vec4(0,0,0,0);
    colour.rgb = doDeferredLight(
       diffuse,
       light,
       specular
    );
    vec3 laser =  texture(lasers, IN.texCoord).xyz;
    if(laser.r >= 0.5f){
    colour.rgb = laser;
    }
    colour.a    = 1.0;

    fragColour =colour;
}
