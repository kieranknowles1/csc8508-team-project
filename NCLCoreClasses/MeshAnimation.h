/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Vector.h"
#include "Matrix.h"

namespace NCL::Rendering {
	using UniqueMeshAnim = std::unique_ptr<class MeshAnimation>;
	using SharedMeshAnim = std::shared_ptr<class MeshAnimation>;

	class MeshAnimation	{
	public:
		MeshAnimation();
		MeshAnimation(size_t jointCount, size_t frameCount, float frameRate, std::vector<Maths::Matrix4>& frames);
		MeshAnimation(const std::string& filename);

		virtual ~MeshAnimation();

		size_t GetJointCount() const {
			return jointCount;
		}

		size_t GetFrameCount() const {
			return frameCount;
		}

		float GetFrameRate() const {
			return frameRate;
		}

		/*float GetFrameTime() const { //removing this for now
			return 1.0f / frameRate;
		}*/

		float GetAnimationTime() const {
			return frameCount / (float)frameRate;
		}

		const Maths::Matrix4* GetJointData(size_t frame) const;

		size_t GetCurrentFrame() const {
			return currentFrame;
		}

		void SetCurrentFrame(size_t frame) {
			currentFrame = frame;
		}

		void UpdateAnimation(float dt); //not sure if this is the best place to put this

	protected:
		size_t		jointCount;
		size_t		frameCount;
		float		frameRate;
		size_t      currentFrame = 0; //added. Could potentially just put this in whatever file I am updating the frame in
		float       frameTime = 0.0f; //added

		std::vector<Maths::Matrix4>		allJoints;
	};
}

