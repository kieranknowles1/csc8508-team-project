#include "BulletDebug.h"

#include <iostream>

#include <CSC8503CoreClasses/Debug.h>
#include <NCLCoreClasses/Maths.h>

namespace NCL::CSC8503 {
	void BulletDebug::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
	{
		Debug::DrawLine(from, to, toNclColor(color));
	}
	void BulletDebug::drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
	{
		Maths::Matrix4 mat = Maths::Matrix::Translation(Vector3(pointOnB));
		Debug::DrawAxisLines(mat, distance, lifeTime);
	}
	void BulletDebug::reportErrorWarning(const char* warningString)
	{
		std::cout << "Bullet Error: " << warningString << std::endl;
	}
	void BulletDebug::draw3dText(const btVector3& location, const char* textString)
	{
		std::cout << "Bullet 3d Text: (" << location << ") " << textString << std::endl;
	}
}