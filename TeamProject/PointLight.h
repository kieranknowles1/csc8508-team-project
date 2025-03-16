#pragma once
#include "Vector.h"
#include "Matrix.h"
#include "Camera.h"
#include "CollisionInfo.h"

#include <btBulletDynamicsCommon.h>

using namespace NCL::CSC8503;

class PointLight {
public:
	PointLight(btVector3 worldPositionIn, float radiusIn,float intensityIn, btVector4 colourIn) {
		worldPosition = worldPositionIn; radius = radiusIn; intensity = intensityIn; colour = colourIn;
	};
	~PointLight() {};

	btVector3 worldPosition;
	float radius;
	float intensity;
	btVector4 colour;
};
