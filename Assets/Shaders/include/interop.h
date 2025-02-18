#ifndef INTEROP_H
#define INTEROP_H
/*
This lets us use the same structs in PSSL, C++, and GLSL
*/

#ifdef __cplusplus
using mat4 = NCL::Maths::Matrix4;

using vec2 = NCL::Maths::Vector2;
using vec3 = NCL::Maths::Vector3;
using vec4 = NCL::Maths::Vector4;
using uint = uint32_t;

#define USING_GL 0
#define USING_PS 0
#define USING_CPP 1
#elif defined(GL_core_profile)
#define USING_GL 1
#define USING_PS 0
#define USING_CPP 0
#else
#define mat4 matrix
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define uint uint32_t

#define USING_GL 0
#define USING_PS 1
#define USING_CPP 0
#endif

#endif

// Setting a struct as an output in glsl causes weird errors, do this instead
#define VERTEX { \
    vec4 position; \
	vec4 colour; \
	vec2 texCoord; \
	vec4 shadowProj; \
	vec3 normal; \
	vec3 worldPos; \
	vec3 tangent; \
	vec3 binormal; \
}
struct Vtx VERTEX;
