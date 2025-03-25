#version 400 core

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

uniform bool hasTexture;
uniform bool isFlat;
uniform bool hasNormalMap; 

in Vertex {
    vec4 colour;
	vec2 texCoord;
	//vec4 shadowProj; temporarily removing shadows
	vec3 normal;
	vec3 worldPos;
	vec3 tangent;
	vec3 binormal;
} IN;

out vec4 fragColour[2];

void main(void)   { 

	if(isFlat){
		fragColour[0] = IN.colour;
		fragColour[1] = vec4(IN.normal.xyz * 0.5 + 0.5, 1.0);
		return;
	}

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

	albedo.a = 1;
	fragColour[0] = albedo; //all the (non-lighting) colour information goes into here
	fragColour[1] = vec4(vecnormal.xyz * 0.5 + 0.5, 1.0); //(THE *0.5 + 0.5) may be unneccessary for floating point textures
	}

	