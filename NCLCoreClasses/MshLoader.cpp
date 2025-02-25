#include "MshLoader.h"
#include "Assets.h"

#include "Vector.h"
#include "Matrix.h"

#include "Maths.h"

#include "Mesh.h"

using namespace NCL;
using namespace Rendering;
using namespace Maths;

bool MshLoader::LoadMesh(const std::string& filename, Mesh& destinationMesh) {
	auto binFile(Assets::MESHDIR + filename + "b");
	if (std::filesystem::exists(binFile)) {
		return LoadBinaryMesh(binFile, destinationMesh);
	}
	else {
		auto path(Assets::MESHDIR + filename);
		std::cout << filename << " is being loaded from text. Consider adding to COMPILED_MESHES" << std::endl;
		return LoadTextMesh(path, destinationMesh);
	}
}

template <typename T>
void read(std::ifstream& fs, T& out) {
	fs.read((char*)&out, sizeof(T));
}

template <typename T>
std::vector<T> readVector(std::ifstream& fs, int size) {
	static_assert(std::is_trivially_copyable<T>::value);
	std::vector<T> out;
	out.resize(size);
	fs.read((char*)out.data(), size * sizeof(T));
	return out;
}

std::vector<std::string> readStrings(std::ifstream& fs, int size) {
	std::vector<std::string> out;
	for (int i = 0; i < size; i++) {
		std::string value;
		char ch;
		while (true) {
			ch = fs.get();
			if (ch != '\0') {
				value += ch;
			}
			else {
				break;
			}

			if (value.length() > 1024) {
				std::cerr << "Unreasonably long string" << std::endl;
				return out;
			}
		}
	}
	return out;
}

bool NCL::Rendering::MshLoader::LoadBinaryMesh(const std::string& filename, Mesh& destinationMesh)
{
	std::ifstream fs(filename, std::ifstream::binary);
	if (!fs.is_open()) {
		std::cerr << __FUNCTION__ << "Could not open mesh " << filename << std::endl;
		return false;
	}

	char magic[4];
	fs.read(magic, 4);
	if (magic[0] != 'M' || magic[1] != 'S' || magic[2] != 'H' || magic[3] != 'B') {
		std::cerr << "Invalid file type" << std::endl;
		return false;
	}

	uint32_t version;
	fs.read((char*)&version, sizeof(version));
	if (version != 1) {
		std::cout << __FUNCTION__ << " Mesh file has incompatible version!\n";
	}

	size_t numMeshes;
	size_t numVertexes;
	size_t numIndexes;
	int numChunks;
	read(fs, numMeshes);
	read(fs, numVertexes);
	read(fs, numIndexes);
	read(fs, numChunks);

	for (int i = 0; i < numChunks; i++) {
		GeometryChunkTypes type;
		read(fs, type);

		switch (type) {
		case GeometryChunkTypes::VPositions: {
			auto vpos = readVector<Vector3>(fs, numVertexes);
			destinationMesh.SetVertexPositions(vpos);
			break;
		} case GeometryChunkTypes::VNormals: {
			auto vnorm = readVector<Vector3>(fs, numVertexes);
			destinationMesh.SetVertexNormals(vnorm);
			break;
		} case GeometryChunkTypes::VTangents: {
			auto vtan = readVector<Vector4>(fs, numVertexes);
			destinationMesh.SetVertexTangents(vtan);
			break;
		} case GeometryChunkTypes::VTex0: {
			auto uv = readVector<Vector2>(fs, numVertexes);
			destinationMesh.SetVertexTextureCoords(uv);
			break;
		} case GeometryChunkTypes::VWeightValues: {
			auto weights = readVector<Vector4>(fs, numVertexes);
			destinationMesh.SetVertexSkinWeights(weights);
		} case GeometryChunkTypes::Indices: {
			auto indexes = readVector<unsigned int>(fs, numIndexes);
			destinationMesh.SetVertexIndices(indexes);
			break;
		} case GeometryChunkTypes::SubMeshes: {
			auto meshes = readVector<SubMesh>(fs, numMeshes);
			destinationMesh.SetSubMeshes(meshes);
			break;
		} case GeometryChunkTypes::SubMeshNames: {
			auto names = readStrings(fs, numMeshes);
			destinationMesh.SetSubMeshNames(names);
			break;
		} default: {
			std::cerr << __FUNCTION__ << " Unknown chunk type " << (int)type << " at offset 0x" << std::hex << fs.tellg() << std::dec << " in " << filename << std::endl;
			return false; // Chunks do not store their length, so we can't recover from an unknown type
		}
		}
	}

	return true;
}

bool MshLoader::LoadTextMesh(const std::string& filename, Mesh& destinationMesh) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << __FUNCTION__ << "Could not open mesh " << filename << std::endl;
	}

	std::string filetype;
	int fileVersion;

	file >> filetype;

	if (filetype != "MeshGeometry") {
		std::cout << __FUNCTION__ << " File is not a Mesh file!\n";
		return false;
	}

	file >> fileVersion;

	if (fileVersion != 1) {
		std::cout << __FUNCTION__ << " Mesh file has incompatible version!\n";
		return false;
	}

	int numMeshes = 0; //read
	int numVertices = 0; //read
	int numIndices = 0; //read
	int numChunks = 0; //read

	file >> numMeshes;
	file >> numVertices;
	file >> numIndices;
	file >> numChunks;

	for (int i = 0; i < numChunks; ++i) {
		int chunkType = (int)GeometryChunkTypes::VPositions;

		file >> chunkType;

		switch ((GeometryChunkTypes)chunkType) {
		case GeometryChunkTypes::VPositions: {
			vector<Vector3> positions;
			ReadTextFloats(file, positions, numVertices);
			destinationMesh.SetVertexPositions(positions);
		}break;
		case GeometryChunkTypes::VColors: {
			vector<Vector4> colours;
			ReadTextFloats(file, colours, numVertices);
			destinationMesh.SetVertexColours(colours);
		}break;
		case GeometryChunkTypes::VNormals: {
			vector<Vector3> normals;
			ReadTextFloats(file, normals, numVertices);
			destinationMesh.SetVertexNormals(normals);
		}break;
		case GeometryChunkTypes::VTangents: {
			vector<Vector4> tangents;
			ReadTextFloats(file, tangents, numVertices);
			destinationMesh.SetVertexTangents(tangents);

		}break;
		case GeometryChunkTypes::VTex0: {
			vector<Vector2> texCoords;
			ReadTextFloats(file, texCoords, numVertices);
			destinationMesh.SetVertexTextureCoords(texCoords);

		}break;
		case GeometryChunkTypes::Indices: {
			vector<unsigned int> indices;
			ReadIntegers(file, indices, numIndices);
			destinationMesh.SetVertexIndices(indices);
		}break;

		case GeometryChunkTypes::VWeightValues: {
			vector<Vector4> skinWeights;
			ReadTextFloats(file, skinWeights, numVertices);
			destinationMesh.SetVertexSkinWeights(skinWeights);
		}break;
		case GeometryChunkTypes::VWeightIndices: {
			vector<Vector4i> skinIndices;
			ReadTextInts(file, skinIndices, numVertices);
			destinationMesh.SetVertexSkinIndices(skinIndices);
		}break;
		case GeometryChunkTypes::JointNames: {
			std::vector<std::string> jointNames;
			ReadJointNames(file, jointNames);
			destinationMesh.SetJointNames(jointNames);
		}break;
		case GeometryChunkTypes::JointParents: {
			vector<int> parents;
			ReadJointParents(file, parents);
			destinationMesh.SetJointParents(parents);
		}break;
		case GeometryChunkTypes::BindPose: {
			vector<Matrix4> bindPose;
			ReadRigPose(file, bindPose);
			destinationMesh.SetBindPose(bindPose);
		}break;
		case GeometryChunkTypes::BindPoseInv: {
			vector<Matrix4> inverseBindPose;
			ReadRigPose(file, inverseBindPose);
			destinationMesh.SetInverseBindPose(inverseBindPose);
		}break;
		case GeometryChunkTypes::SubMeshes: {
			vector<SubMesh> subMeshes;
			ReadSubMeshes(file, numMeshes, subMeshes);

			destinationMesh.SetSubMeshes(subMeshes);
		}break;
		case GeometryChunkTypes::SubMeshNames: {
			std::vector<std::string> subMeshNames;
			ReadSubMeshNames(file, numMeshes, subMeshNames);
			destinationMesh.SetSubMeshNames(subMeshNames);
		}break;
		}
	}

	if (!destinationMesh.GetBindPose().empty() && destinationMesh.GetInverseBindPose().empty()) {
		destinationMesh.CalculateInverseBindPose();
	}

	destinationMesh.SetPrimitiveType(GeometryPrimitive::Triangles);

	return true;
}

void MshLoader::ReadRigPose(std::ifstream& file, vector<Matrix4>& into) {
	int matCount = 0;
	file >> matCount;

	for (int m = 0; m < matCount; ++m) {
		Matrix4 mat;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				file >> mat.array[i][j];
			}
		}

		into.emplace_back(mat);
	}
}

void MshLoader::ReadJointParents(std::ifstream& file, std::vector<int>& parentIDs) {
	int jointCount = 0;
	file >> jointCount;

	for (int i = 0; i < jointCount; ++i) {
		int id = -1;
		file >> id;
		parentIDs.emplace_back(id);
	}
}

void MshLoader::ReadJointNames(std::ifstream& file, std::vector<std::string>& jointNames) {
	int jointCount = 0;
	file >> jointCount;
	std::string jointName;
	std::getline(file, jointName);

	for (int i = 0; i < jointCount; ++i) {
		std::string jointName;
		std::getline(file, jointName);
		jointNames.emplace_back(jointName);
	}
}

void MshLoader::ReadSubMeshes(std::ifstream& file, int count, std::vector<SubMesh>& subMeshes) {
	for (int i = 0; i < count; ++i) {
		SubMesh m;
		file >> m.start;
		file >> m.count;
		subMeshes.emplace_back(m);
	}
}

void MshLoader::ReadSubMeshNames(std::ifstream& file, int count, std::vector<std::string>& subMeshNames) {
	std::string scrap;
	std::getline(file, scrap);

	for (int i = 0; i < count; ++i) {
		std::string meshName;
		std::getline(file, meshName);
		subMeshNames.emplace_back(meshName);
	}
}

void* MshLoader::ReadVertexData(GeometryChunkData dataType, GeometryChunkTypes chunkType, int numVertices) {
	int numElements = 3;

	if (chunkType == GeometryChunkTypes::VTex0 ||
		chunkType == GeometryChunkTypes::VTex1) {
		numElements = 2;
	}
	else if (chunkType == GeometryChunkTypes::VColors) {
		numElements = 4;
	}

	int bytesPerElement = 4;

	if (dataType == GeometryChunkData::dShort) {
		bytesPerElement = 2;
	}
	else if (dataType == GeometryChunkData::dByte) {
		bytesPerElement = 1;
	}

	char* data = new char[numElements * bytesPerElement];

	return data;
}

void MshLoader::ReadTextInts(std::ifstream& file, vector<Vector2i>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector2i temp;
		file >> temp[0];
		file >> temp[1];
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTeReadTextIntsxtFloats(std::ifstream& file, vector<Vector3i>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector3i temp;
		file >> temp[0];
		file >> temp[1];
		file >> temp[2];
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTextInts(std::ifstream& file, vector<Vector4i>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector4i temp;
		file >> temp[0];
		file >> temp[1];
		file >> temp[2];
		file >> temp[3];
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector2>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector2 temp;
		file >> temp.x;
		file >> temp.y;
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector3>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector3 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		element.emplace_back(temp);
	}
}

void MshLoader::ReadTextFloats(std::ifstream& file, vector<Vector4>& element, int numVertices) {
	for (int i = 0; i < numVertices; ++i) {
		Vector4 temp;
		file >> temp.x;
		file >> temp.y;
		file >> temp.z;
		file >> temp.w;
		element.emplace_back(temp);
	}
}

void MshLoader::ReadIntegers(std::ifstream& file, vector<unsigned int>& elements, int intCount) {
	for (int i = 0; i < intCount; ++i) {
		unsigned int temp;
		file >> temp;
		elements.emplace_back(temp);
	}
}

void MshLoader::WriteTextFloats(std::ofstream& file, const vector<Maths::Vector2>& elements) {
	for(const auto& v : elements) {
		file << v.x << " " << v.y << "\n";
	}
}

void MshLoader::WriteTextFloats(std::ofstream& file, const vector<Maths::Vector3>& elements) {
	for (const auto& v : elements) {
		file << v.x << " " << v.y << " " << v.z << "\n";
	}
}

void MshLoader::WriteTextFloats(std::ofstream& file, const vector<Maths::Vector4>& elements) {
	for (const auto& v : elements) {
		file << v.x << " " << v.y << " " << v.z << " " << v.w << "\n";
	}
}

void MshLoader::WriteIntegers(std::ofstream& file, const vector<unsigned int>& elements) {
	for (const auto& i : elements) {
		file << i << "\n";
	}
}

void MshLoader::WriteIntegers(std::ofstream& file, const vector<Maths::Vector4i>& elements) {
	for (const auto& v : elements) {
		file << v.x << " " << v.y << " " << v.z << " " << v.w << "\n";
	}
}

void MshLoader::WriteMatrices(std::ofstream& file, const vector<Matrix4>& elements) {
	for (const auto& m : elements) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				file << m.array[i][j] << " ";
			}
		}
		file << "\n";
	}
}

void MshLoader::WriteSubMeshes(std::ofstream& file, const vector<struct SubMesh>& elements) {
	for (const auto& m : elements) {
		file << m.start << " " << m.count << "\n";
	}
}
