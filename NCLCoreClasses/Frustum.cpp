/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "Frustum.h"
#include "Vector.h"
#include "Matrix.h"

using namespace NCL;
using namespace NCL::Maths;

Frustum::Frustum(void) {

};

Frustum Frustum::FromViewProjMatrix(const Matrix4& mat, float ndcNear, float ndcFar) {
	Frustum f;
	float* values = (float*)&mat;

	// Extract each axis from the matrix
	// then add/subtract to tilt the normals
	Vector3 xaxis = Vector3(values[0], values[4], values[8]);
	Vector3 yaxis = Vector3(values[1], values[5], values[9]);
	Vector3 zaxis = Vector3(values[2], values[6], values[10]);
	Vector3 waxis = Vector3(values[3], values[7], values[11]);

	// Right
	f.planes[0] = Plane(
		waxis - xaxis,
		values[15] - values[12],
		true
	);

	// Left
	f.planes[1] = Plane(
		waxis + xaxis,
		values[15] + values[12],
		true
	);

	// Bottom
	f.planes[2] = Plane(
		waxis + yaxis,
		values[15] + values[13],
		true
	);

	// Top
	f.planes[3] = Plane(
		waxis - yaxis,
		values[15] - values[13],
		true
	);

	// Far
	f.planes[4] = Plane(
		waxis - zaxis,
		values[15] - values[14],
		true
	);

	// Near
	f.planes[5] = Plane(
		waxis + zaxis,
		values[15] + values[14],
		true
	);

	return f;
}