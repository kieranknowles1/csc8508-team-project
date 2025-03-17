#version 400 core

#include "include/vert/texscale.glsl" 

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
//added for normal mapping:
layout(location = 4) in vec4 tangent; //location 4 as per vertex attributes enum in Mesh.h

uniform vec4 objectColour;

uniform bool hasVertexColours = false;
uniform bool isFlat = false;
uniform vec3 texScale;
uniform bool  texRepeating;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	//vec4 shadowProj; temporarily removing shadows
	vec3 normal;
	vec3 worldPos;
	//Added for normal mapping:
	vec3 tangent;
	vec3 binormal;
} OUT;

void main(void)
{
	// FIXME: Use a texture matrix, not this hack
	// OUT.texCoord = scaleUv(texCoord, texScale, normal, texRepeating);1
	OUT.texCoord = texCoord;
	OUT.texCoord.y = 1.0 - OUT.texCoord.y; //flip y axis

	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));

	//OUT.shadowProj 	=  shadowMatrix * vec4 ( position,1); TEMPORARY
	OUT.worldPos 	= ( modelMatrix * vec4 ( position ,1)). xyz ;
	OUT.normal 		= normalize ( normalMatrix * normalize ( normal ));
	OUT.tangent     = normalize ( normalMatrix * normalize ( tangent.xyz)); //calculate tangent for normal mapping
	OUT.binormal    = cross ( OUT.tangent, OUT.normal) * tangent.w; //calculate the binormal for normal mapping
	OUT.colour		= objectColour;
	gl_Position		= mvp * vec4(position, 1.0);
}
