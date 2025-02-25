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
static void writeOptionalChunk(int& chunksWritten, std::ofstream& fs, MshLoader::GeometryChunkTypes type, const std::vector<T>& data, int expectedSize) {
	if (data.empty()) return;
	chunksWritten++;

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
static int writeOptionalJointNames(int& chunksWritten, std::ofstream& fs, const std::vector<std::string>& data) {
	if (data.empty()) return 0;
	chunksWritten++;
	
	write(fs, MshLoader::GeometryChunkTypes::JointNames);
	write(fs, data.size());

	writeStrings(fs, data);

	return data.size();
}

static void writeOptionalJointParents(int& chunksWritten, std::ofstream& fs, const std::vector<int>& data, int expectedSize) {
	if (data.empty()) return;
	chunksWritten++;
	checkSize(expectedSize, data.size());

	write(fs, MshLoader::GeometryChunkTypes::JointParents);
	write(fs, data.size());

	for (auto v : data) {
		write(fs, v);
	}
}

static void writeOptionalRigPose(int& chunksWritten, std::ofstream& fs, MshLoader::GeometryChunkTypes type, const std::vector<Matrix4>& data) {
	if (data.empty()) return;
	chunksWritten++;

	write(fs, type);
	write(fs, data.size());

	for (auto& v : data) {
		write(fs, v);
	}
}

static void writeOptionalSubMeshNames(int& chunksWritten, std::ofstream& fs, const std::vector<std::string>& data, int expectedCount) {
	if (data.empty()) return;
	chunksWritten++;
	checkSize(expectedCount, data.size());

	write(fs, MshLoader::GeometryChunkTypes::SubMeshNames);
	writeStrings(fs, data);
}

// Binary mesh format:
// - MSHB magic
// - Rest of data in text mesh format, only converted to binary. Single precision floats and 32-bit integers

static void convert(std::string_view source, std::string_view target) {
	Mesh mesh;
	bool ok = MshLoader::LoadTextMesh(std::string(source), mesh);
	if (!ok) {
		throw std::runtime_error("Failed to load mesh");
	}

	std::ofstream fs(std::string(target), std::ios::binary);
	fs.write("MSHB", 4);
	write(fs, Version);
	write(fs, mesh.GetSubMeshCount());
	write(fs, mesh.GetVertexCount());
	write(fs, mesh.GetIndexCount());
	auto chunkCountOff = fs.tellp();
	write(fs, 0); // Will be filled later

	int chunksWritten = 0;
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VPositions, mesh.GetPositionData(), mesh.GetVertexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VColors, mesh.GetColourData(), mesh.GetVertexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VNormals, mesh.GetNormalData(), mesh.GetVertexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VTangents, mesh.GetTangentData(), mesh.GetVertexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VTex0, mesh.GetTextureCoordData(), mesh.GetVertexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::Indices, mesh.GetIndexData(), mesh.GetIndexCount());
	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::VWeightValues, mesh.GetSkinWeightData(), mesh.GetVertexCount());
	int jointNameCount = writeOptionalJointNames(chunksWritten, fs, mesh.GetJointNames());
	writeOptionalJointParents(chunksWritten, fs, mesh.GetJointParents(), jointNameCount);

	writeOptionalRigPose(chunksWritten, fs, MshLoader::GeometryChunkTypes::BindPose, mesh.GetBindPose());
	writeOptionalRigPose(chunksWritten, fs, MshLoader::GeometryChunkTypes::BindPoseInv, mesh.GetInverseBindPose());

	writeOptionalChunk(chunksWritten, fs, MshLoader::GeometryChunkTypes::SubMeshes, mesh.GetSubMeshes(), mesh.GetSubMeshCount());
	writeOptionalSubMeshNames(chunksWritten, fs, mesh.GetSubMeshNames(), mesh.GetSubMeshCount());

	fs.seekp(chunkCountOff);
	write(fs, chunksWritten);
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