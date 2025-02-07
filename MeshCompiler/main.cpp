#include <string>
#include <fstream>
#include <iostream>

#include <NCLCoreClasses/Mesh.h>
#include <NCLCoreClasses/MshLoader.h>

using namespace NCL::Rendering;

const static constexpr uint32_t Version = 1;

static void checkSize(int expected, int actual) {
	if (expected != actual) {
		throw std::runtime_error("Actual chunk size did not match expected");
	}
}

template <typename T>
static void write(std::ofstream& fs, T data) {
	fs.write(reinterpret_cast<char*>(&data), sizeof(T));
}

template <typename T>
static void writeOptionalChunk(std::ofstream& fs, MshLoader::GeometryChunkTypes type, const std::vector<T>& data, int expectedSize) {
	if (data.empty()) return;

	checkSize(expectedSize, data.size());

	write(fs, type);
	for (auto& v : data) {
		write(fs, v);
	}
}

static void writeStrings(std::ofstream& fs, const std::vector<std::string>& data) {
	for (auto& v : data) {
		fs.write(v.data(), v.length());
		fs.write("\0", 1); // Null terminator
	}
}

// Returns number of names written
static int writeOptionalJointNames(std::ofstream& fs, const std::vector<std::string>& data) {
	if (data.empty()) return 0;
	
	write(fs, MshLoader::GeometryChunkTypes::JointNames);
	write(fs, data.size());

	writeStrings(fs, data);

	return data.size();
}

static void writeOptionalJointParents(std::ofstream& fs, const std::vector<int>& data, int expectedSize) {
	if (data.empty()) return;
	checkSize(expectedSize, data.size());

	write(fs, MshLoader::GeometryChunkTypes::JointParents);
	write(fs, data.size());

	for (auto v : data) {
		write(fs, v);
	}
}

static void writeOptionalRigPose(std::ofstream& fs, MshLoader::GeometryChunkTypes type, const std::vector<Matrix4>& data) {
	if (data.empty()) return;

	write(fs, type);
	write(fs, data.size());

	for (auto& v : data) {
		write(fs, v);
	}
}

static void writeOptionalSubMeshNames(std::ofstream& fs, const std::vector<std::string>& data, int expectedCount) {
	if (data.empty()) return;
	checkSize(expectedCount, data.size());

	writeStrings(fs, data);
}

// Binary mesh format:
// - MSHB magic
// - Rest of data in text mesh format, only converted to binary. Single precision floats and 32-bit integers

static void convert(std::string_view source, std::string_view target) {
	Mesh mesh;
	bool ok = MshLoader::LoadMesh(std::string(source), mesh);
	if (!ok) {
		throw std::runtime_error("Failed to load mesh");
	}

	std::ofstream fs(std::string(target), std::ios::binary);
	fs.write("MSHB", 4);
	write(fs, Version);
	write(fs, mesh.GetSubMeshCount());
	write(fs, mesh.GetVertexCount());
	write(fs, mesh.GetIndexCount());
	write(fs, mesh.numChunks);

	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VPositions, mesh.GetPositionData(), mesh.GetVertexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VColors, mesh.GetColourData(), mesh.GetVertexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VNormals, mesh.GetNormalData(), mesh.GetVertexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VTangents, mesh.GetTangentData(), mesh.GetVertexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VTex0, mesh.GetTextureCoordData(), mesh.GetVertexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::Indices, mesh.GetIndexData(), mesh.GetIndexCount());
	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::VWeightValues, mesh.GetSkinWeightData(), mesh.GetVertexCount());
	int jointNameCount = writeOptionalJointNames(fs, mesh.GetJointNames());
	writeOptionalJointParents(fs, mesh.GetJointParents(), jointNameCount);

	writeOptionalRigPose(fs, MshLoader::GeometryChunkTypes::BindPose, mesh.GetBindPose());
	writeOptionalRigPose(fs, MshLoader::GeometryChunkTypes::BindPoseInv, mesh.GetInverseBindPose());

	writeOptionalChunk(fs, MshLoader::GeometryChunkTypes::SubMeshes, mesh.GetSubMeshes(), mesh.GetSubMeshCount());
	writeOptionalSubMeshNames(fs, mesh.GetSubMeshNames(), mesh.GetSubMeshCount());
}

int main(int argc, char** argv) {
	if (argc != 3) { // process name + source + target
		std::cerr << "Usage: " << argv[0] << " [source] [target]" << std::endl;
		return 1;
	}

	std::string_view source(argv[1]);
	std::string_view target(argv[2]);
	std::cout << "Converting " << source << " to " << target << std::endl;

	convert(source, target);
}