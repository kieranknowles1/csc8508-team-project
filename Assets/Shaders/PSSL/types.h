#ifndef PSSL_TYPES_H
#define PSSL_TYPES_H

#include "interop.h"

#define NULLTEX -1

struct ObjectState {
		matrix modelMatrix;
		float4 colour;
		int index[4];
		bool texRepeats;
		float3 texScale;
};

struct UiState {
    float4 colour;
    float2 position;
    float2 size;
    int texture;
};

#endif
