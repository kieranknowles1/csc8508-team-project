#ifndef PSSL_TYPES_H
#define PSSL_TYPES_H

#include "glinterop.h"
#include "../include/types.h"

#ifdef __cplusplus
#define SLOT(slot)
#else
#define SLOT(slot) : slot
#endif

#define NULLTEX -1

struct ObjectState {
		matrix modelMatrix;
		// transpose(inverse(mat3(modelMatrix)))
		// HLSL doesn't have an inverse function, so doing this in cpp
		matrix normalMatrix;
		float4 colour;
		int texIndex = NULLTEX;
		int normalIndex = NULLTEX;
		int skinningIndex = NULLTEX;
		bool texRepeats;
		float3 texScale;
		int startIndex;
		int numElements;
		bool invertY;
};

struct UiState {
    float4 colour;
    float2 position;
    float2 size;
    int texture = NULLTEX;
};

struct TextState {
	float2 pos;
	float2 uv;
	float4 colour;
};

struct LineState {
	float4 pos;
	float4 colour;
};

struct LaserState {
	float3 start;
	int __pad1;
	float3 end;
	float thickness;
	float4 colour;
};

struct DecalState {
	matrix modelMatrix;
	float4 color;
	int textureId;
	float fade;
};

struct VS_OUTPUT
{
	float4 Position SLOT(S_POSITION);
	float2 UV SLOT(TEXCOORD0);
	float4 Colour SLOT(TEXCOORD1);
	int	texID SLOT(TEXCOORD2);
	int normId SLOT(TEXCOORD3);

	float3 tangent;
	float3 normal;
	float3 binormal;
};

struct UI_VS_OUTPUT {
    float4 position SLOT(S_POSITION);
    float2 UV SLOT(TEXCOORD0);
    float4 colour SLOT(TEXCOORD1);
    int texId SLOT(TEXCOORD2);
};

struct DEFERRED_VS_OUTPUT
{
	float4 position SLOT(S_POSITION);
	int stateId SLOT(TEXCOORD0);
};

struct TEXT_VS_OUTPUT
{
	float4 Position SLOT(S_POSITION);
	float2 UV SLOT(TEXCOORD0);
	float4 Colour SLOT(TEXCOORD1);
};

struct LINE_VS_OUTPUT
{
	float4 Position SLOT(S_POSITION);
	float4 Colour SLOT(TEXCOORD1);
};

struct LASER_VS_OUTPUT
{
	float4 position SLOT(S_POSITION);
	float4 color SLOT(TEXCOORD0);
	float depth SLOT(TEXCOORD1);
};

struct DECAL_VS_OUTPUT
{
	float4 position SLOT(S_POSITION);
	float4 color SLOT(TEXCOORD0);
	float2 uv SLOT(TEXCOORD1);
	int texId SLOT(TEXCOORD2);
	float depth SLOT(TEXCOORD3);
};

#undef SLOT
#endif
