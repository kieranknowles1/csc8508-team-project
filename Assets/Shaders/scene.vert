#version 400 core

#include "include/interop.h"
#include "include/constants.glh"

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform vec4 objectColour;

uniform bool hasVertexColours = false;
uniform bool isFlat = false;
uniform float texScaleX;
uniform float texScaleY;
uniform float texScaleZ;
uniform bool  texRepeating;

#include "impl/scene.vert"

out Vertex VERTEX OUT;
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
//added for normal mapping:
layout(location = 4) in vec4 tangent; //location 4 as per vertex attributes enum in Mesh.h

void main(void)
{
	Vtx vert = doSceneVertex(position, colour, texCoord, normal, tangent);
	OUT.position = vert.position;
	gl_Position = vert.position;
	OUT.colour = vert.colour;
	OUT.texCoord = vert.texCoord;
	OUT.shadowProj = vert.shadowProj;
	OUT.normal = vert.normal;
	OUT.worldPos = vert.worldPos;
	OUT.tangent = vert.tangent;
	OUT.binormal = vert.binormal;
}
