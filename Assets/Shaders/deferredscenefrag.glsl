#version 330 core //Basically like scene.frag but without the lighting calculations

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
	//Added for normal mapping:
	vec3 tangent;
	vec3 binormal;
} IN;

out vec4 fragColour[2];

void main(void)   { 
  /*  mat3 TBN = mat3(normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));

	vec3 normal = texture2D(normalTex, IN.texCoord).rgb * 2.0 - 1.0; //just this from deferred rendering initially
	normal      = normalize(TBN * normalize(normal));
	*/

	if(isFlat){
		fragColour[0] = IN.colour;
		fragColour[1] = vec4(IN.normal.xyz * 0.5 + 0.5, 1.0);
		return;
	}

	vec4 albedo = IN.colour;
	if(hasTexture) {
	 albedo *= texture(diffuseTex, IN.texCoord); 
	}

	vec3 normal;

	vec3 mapnormal;
	 if(hasNormalMap) { //this version of normal calculations is more in line with scene.frag
	    mat3 TBN = mat3 (normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
		mapnormal = texture(normalTex, IN.texCoord).rgb; 
		mapnormal = normalize(TBN * normalize(mapnormal * 2.0 - 1.0)); 
	    normal = mapnormal;
	}

	fragColour[0] = albedo; //all the (non-lighting) colour information goes into here
	fragColour[1] = vec4(normal.xyz * 0.5 + 0.5, 1.0); 
	}