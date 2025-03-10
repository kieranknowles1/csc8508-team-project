#ifndef PSSL_TYPES_H
#define PSSL_TYPES_H

#ifdef __cplusplus
#define SLOT(slot)
#else
#define SLOT(slot) : slot
#endif

#include "interop.h"

#define NULLTEX -1

struct ObjectState {
		matrix modelMatrix;
		float4 colour;
		int texIndex = NULLTEX;
		int skinningIndex = NULLTEX;
		bool texRepeats;
		float3 texScale;
};

struct UiState {
    float4 colour;
    float2 position;
    float2 size;
    int texture = NULLTEX;
};

struct VS_OUTPUT
{
	float4 Position SLOT(S_POSITION);
	float2 UV SLOT(TEXCOORD0);
	float4 Colour SLOT(TEXCOORD1);
	int	texID SLOT(TEXCOORD2);

	float4 ShadowClip SLOT(TEXCOORD2);
};

struct UI_VS_OUTPUT {
    float4 position SLOT(S_POSITION);
    float2 UV SLOT(TEXCOORD0);
    float4 colour SLOT(TEXCOORD1);
    int texId SLOT(TEXCOORD2);
};

struct SHADOW_VS_OUTPUT
{
	float4 Position SLOT(POSITION);
	float2 UV SLOT(TEXCOORD0);
	float4 Colour SLOT(TEXCOORD1);
	int	texID SLOT(TEXCOORD2);
};

#endif
