#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "CollisionInfo.h"

#include <btBulletDynamicsCommon.h>

using namespace NCL::CSC8503;

class PointLight {
public:
	PointLight(btVector3 worldPositionIn, float radiusIn, btVector4 colourIn) {
		worldPosition = worldPositionIn; radius = radiusIn; colour = colourIn;
	};
	~PointLight();

	btVector3 worldPosition;
	float radius;
	btVector4 colour;
};
