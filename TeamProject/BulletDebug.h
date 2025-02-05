#pragma once

#include <LinearMath/btIDebugDraw.h>

#include <NCLCoreClasses/Vector.h>

namespace NCL::CSC8503 {
	// Implementation of a Bullet debug renderer
	// Acts as a wrapper for NCL::Debug
	class BulletDebug : public btIDebugDraw {
	private:
		int debugMode = Disabled;

		Maths::Vector4 toNclColor(const btVector3& color) {
			return Maths::Vector4(color.x(), color.y(), color.z(), 1.0f);
		}

	public:
		static constexpr DebugDrawModes Disabled = DBG_NoDebug;
		static constexpr DebugDrawModes Enabled = DBG_DrawWireframe;

		void toggle() {
			debugMode = (debugMode == Disabled) ? Enabled : Disabled;
		}

		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

		void drawContactPoint(const btVector3& pointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;

		void reportErrorWarning(const char* warningString) override;

		void draw3dText(const btVector3& location, const char* textString) override;

		void setDebugMode(int debugMode) override { this->debugMode = debugMode; }

		int getDebugMode() const override { return debugMode; }
	};
}