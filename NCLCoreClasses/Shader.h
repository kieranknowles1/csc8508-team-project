/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

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

	class Shader {
	public:
		struct Key {
			std::string vertex;
			std::string fragment;
			Key(std::string_view vertex, std::string_view fragment) : vertex(vertex), fragment(fragment) {}

			inline friend std::ostream& operator<<(std::ostream& o, const Shader::Key& v) {
				o << "Vertex(" << v.vertex << ") Fragment(" << v.fragment << ")";
				return o;
			}

			bool operator==(const Key& rhs) const {
				return vertex == rhs.vertex && fragment == rhs.fragment;
			}
			bool operator<(const Key& rhs) const {
				if (vertex < rhs.vertex) return true;
				return fragment < rhs.fragment;
			}
		};

		const static Key Default;

		Shader() {
			assetID = 0;
		}
		Shader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		virtual ~Shader();

		uint32_t GetAssetID() const {
			return assetID;
		}

		void SetAssetID(uint32_t newID) {
			assetID = newID;
		}

		virtual void ReloadShader() = 0;
	protected:
		std::string		shaderFiles[ShaderStages::MAX_SIZE];
		uint32_t		assetID;
	};
}
