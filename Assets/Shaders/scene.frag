#version 400 core

#include "include/interop.h"
#include "include/constants.glh"

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
in Vertex VERTEX IN;

#include "impl/scene.frag"

out vec4 fragColor;

void main(void)
{
	Vtx inp;
	inp.position = IN.position;
	inp.colour = IN.colour;
	inp.texCoord = IN.texCoord;
	inp.shadowProj = IN.shadowProj;
	inp.normal = IN.normal;
	inp.worldPos = IN.worldPos;
	inp.tangent = IN.tangent;
	inp.binormal = IN.binormal;
	fragColor = doSceneFrag(inp);
}
