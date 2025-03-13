#version 330 core
uniform sampler2D diffuseTex;
uniform sampler2D diffuseLight;
uniform sampler2D specularLight;
uniform sampler2D diffuseLight2;
uniform sampler2D specularLight2;

in Vertex {
    vec2 texCoord;
} IN;

out vec4 fragColour;

void main(void) {
     vec3 diffuse  = texture(diffuseTex, IN.texCoord).xyz;
     vec3 light    = texture(diffuseLight, IN.texCoord).xyz;
     vec3 specular = texture(specularLight, IN.texCoord).xyz;
     //vec3 light    = texture(diffuseLight, IN.texCoord).xyz + texture(diffuseLight2, IN.texCoord).xyz;
     //vec3 specular = texture(specularLight, IN.texCoord).xyz + texture(specularLight2, IN.texCoord).xyz;
     fragColour.xyz  = diffuse * 0.05; //ambient
     fragColour.xyz += diffuse * light; //lambert
     fragColour.xyz += specular; 
    // fragColour.rgb = pow(fragColour.rgb, vec3(1.0 / 2.2f)); //gamma correction, maybe should be in another shader. Probably should be done last   
     fragColour.a    = 1.0;
}
