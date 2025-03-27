#version 400 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;
uniform sampler2D glossTex;
uniform sampler2D specularTex;

uniform bool hasTexture;
uniform bool isFlat;
uniform bool hasNormalMap; 
uniform bool hasGloss;
uniform bool hasSpecular;

in Vertex {
    vec4 colour;
	vec2 texCoord;
	//vec4 shadowProj; temporarily removing shadows
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal;
} IN;

out vec4 fragColour[4];

void main(void)   {

	vec4 albedo = IN.colour;
	if(hasTexture) {
	 albedo *= texture(diffuseTex, IN.texCoord); 
	 //albedo.rgb = pow(albedo.rgb, vec3(2.2));  //added to transform textures into linear space to later be gamma corrected
	}

	vec3 vecnormal = IN.normal.xyz; //if no normal map then use the vertex normals
	vec3 mapnormal;
	 if(hasNormalMap) { 
	    mat3 TBN = mat3 (normalize(IN.tangent),normalize(IN.binormal), normalize(IN.normal));
		mapnormal = texture(normalTex, IN.texCoord).rgb * 2.0 - 1.0; 
		mapnormal = normalize(TBN * normalize(mapnormal));
	    vecnormal = mapnormal; //the * 2.0 - 1.0 part converts from the texture space of 0.0 to 1.0 over to vector coordinates ranging from -1.0 to 1.0 so this part is still required
	}

	float gloss = hasGloss ? texture(glossTex, IN.texCoord).r : 1.0;
    float specular = hasSpecular ? texture(specularTex, IN.texCoord).r : 1.0;

    // Clamp values to prevent artifacts
    gloss = clamp(gloss, 0.05, 1.0);        // Avoid zero gloss
    specular = clamp(specular, 0.05, 1.0);  // Avoid zero specular

	// Code I am messing with to get specular and gloss map to work
    fragColour[0] = albedo; // Albedo (diffuse color)
	fragColour[1] = vec4(vecnormal.xyz * 0.5 + 0.5, 1.0); // Normal in RGB, alpha reserved
	fragColour[2] = vec4(vec3(specular), 1.0);
	fragColour[3] = vec4(vec3(gloss), 1.0);
	

// 	fragColour[0].rgb = albedo.rgb; //all the (non-lighting) colour information goes into here
// 	fragColour[0].a = specular;
// 	fragColour[1] = vec4(vecnormal.xyz * 0.5 + 0.5, 1.0); //(THE *0.5 + 0.5) may be unneccessary for floating point textures
}