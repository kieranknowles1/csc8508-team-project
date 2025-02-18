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

#define USING_GL 0
#define USING_PS 1
#define USING_CPP 0
#endif

#endif
