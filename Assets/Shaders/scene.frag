#version 400 core

uniform vec4 		objectColour;
uniform sampler2D 	mainTex;
uniform sampler2DShadow shadowTex;
uniform sampler2D   normalTex;

uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;
uniform bool isFlat;
uniform bool hasNormalMap; //added for normal maps
uniform bool lightAttenuates = false; //added to optionally account for attenuation

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	vec3 tangent; //added for normal mapping
	vec3 binormal; //added for normal mapping
} IN;

out vec4 fragColor;

void main(void)
{
	if(isFlat){
		fragColor = IN.colour;
		return;
	}
	float shadow = 1.0; // New !
	
	if( IN . shadowProj . w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}

	mat3 TBN; //added
	vec3 mapnormal; //added
	vec3 NORMAL = IN.normal; //added

    if(hasNormalMap) { //normal map additions
	    mat3 TBN = mat3 (normalize(IN.tangent), normalize(IN.binormal), normalize(IN.normal));
		mapnormal = texture(normalTex, IN.texCoord).rgb; 
		mapnormal = normalize(TBN * normalize(mapnormal * 2.0 - 1.0)); 
		NORMAL = mapnormal;
		}

	vec3  incident = normalize ( lightPos - IN.worldPos );
	float lambert  = max (0.0 , dot ( incident , NORMAL )) * 0.9; //changed all subsequent IN.normal to NORMAL
	
	vec3 viewDir = normalize ( cameraPos - IN . worldPos );
	vec3 halfDir = normalize ( incident + viewDir );

	float rFactor = max (0.0 , dot ( halfDir , NORMAL ));
	float sFactor = pow ( rFactor , 80.0 );
	
	vec4 albedo = IN.colour;
	
	if(hasTexture) {
	 albedo *= texture(mainTex, IN.texCoord); 
	}

	float attenuation = 1.0f; //default attenuation value 

	if(lightAttenuates) { //added to optionally account for attenuation
	    float distance = length(lightPos - IN.worldPos);
		attenuation = 1.0f - clamp(distance/lightRadius, 0.0, 1.0); 
	}
	
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
	fragColor.rgb = albedo.rgb * 0.05f; //ambient
	
	fragColor.rgb += albedo.rgb * lightColour.rgb * lambert * shadow * attenuation; //diffuse light
	
	fragColor.rgb += lightColour.rgb * sFactor * shadow; //specular light
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	
	fragColor.a = 1;

}