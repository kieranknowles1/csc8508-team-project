#version 330 core

#include "include/post/deferred.glsl"

uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
    vec3 diffuse  = texture(diffuseTex, IN.texCoord).xyz;
    vec3 light    = texture(diffuseLight, IN.texCoord).xyz;
    vec3 specular = texture(specularLight, IN.texCoord).xyz;

    // fragColour.rgb = pow(fragColour.rgb, vec3(1.0 / 2.2f)); //gamma correction, maybe should be in another shader. Probably should be done last
    fragColour.rgb = doDeferredLight(
       diffuse,
       light,
       specular
    );
    fragColour.a    = 1.0;
}
