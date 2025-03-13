#ifndef PSSL_INTEROP_H
#define PSSL_INTEROP_H

/*
This lets us use the same structs in both PSSL and C++
*/

#ifdef __cplusplus
using matrix = NCL::Maths::Matrix4;
using float3x3 = NCL::Maths::Matrix3;

using float2 = NCL::Maths::Vector2;
using float3 = NCL::Maths::Vector3;
using float4 = NCL::Maths::Vector4;

#endif

#endif
