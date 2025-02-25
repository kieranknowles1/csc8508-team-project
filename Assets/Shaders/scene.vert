#version 400 core

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
uniform float texScaleX;
uniform float texScaleY;
uniform float texScaleZ;
uniform bool  texRepeating;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
	//Added for normal mapping:
	vec3 tangent;
	vec3 binormal; 
} OUT;

void main(void)
{
    vec2 TEXCOORD = texCoord; 
	//before applying mvp matrix, vertices are in local space and therefore axis aligned
	//We check to see what axis the plane in question is facing before determining how to scale the texCoords
	//Scaling currently works well for cuboids but not too well for curved surfaces like capsules and spheres
	vec3 direction = normalize(normal);
	if (abs(direction.x) > 0.001 && abs(direction.y) < 0.001 && abs(direction.z) < 0.001) {//face is perpendicular to x plane => on y-z plane
	   
	    TEXCOORD.x *= texScaleZ;
	    TEXCOORD.y *= texScaleY;
    }
	if (abs(direction.y) > 0.001 && abs(direction.x) < 0.001 && abs(direction.z) < 0.001) {//face is perpendicular to y plane => on x-z plane
	 
	    TEXCOORD.x *= texScaleX;
	    TEXCOORD.y *= texScaleZ;
    }
	if (abs(direction.z) > 0.001 && abs(direction.x) < 0.001 && abs(direction.y) < 0.001) {//face is perpendicular to z plane => on x-y plane
	    TEXCOORD.x *= texScaleX;
	    TEXCOORD.y *= texScaleY;
	   }
    if (!texRepeating) { //if texture not to be repeated or scaled then just take original texture coordinates
	   TEXCOORD = texCoord;
	   }

	   OUT.texCoord = TEXCOORD; 

	mat4 mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));

	OUT.shadowProj 	=  shadowMatrix * vec4 ( position,1);
	OUT.worldPos 	= ( modelMatrix * vec4 ( position ,1)). xyz ;
	OUT.normal 		= normalize ( normalMatrix * normalize ( normal ));
	OUT.tangent     = normalize ( normalMatrix * normalize ( tangent.xyz)); //calculate tangent for normal mapping
	OUT.binormal    = cross ( OUT.tangent, OUT.normal) * OUT.tangent; //calculate the binormal for normal mapping
	OUT.colour		= objectColour;
	gl_Position		= mvp * vec4(position, 1.0);
}