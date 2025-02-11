/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

#include <string>
#include "Matrix.h"

namespace NCL::Rendering {
	namespace ShaderStages {
		enum Type : uint32_t {
			Vertex,
			Fragment,
			Geometry,
			Domain,
			Hull,
			Mesh,
			Task,
			MAX_SIZE,
			//Aliases
			TessControl = Domain,
			TessEval = Hull,
		};
	};

	class Shader	{
	public:
		Shader() {}
		Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		virtual ~Shader();

		virtual unsigned int GetProgramID() const = 0;

		virtual void ReloadShader() = 0;

		virtual void setUniform(const char*, const Maths::Matrix4& value) = 0;
	protected:
		std::string		shaderFiles[ShaderStages::MAX_SIZE];
	};
}