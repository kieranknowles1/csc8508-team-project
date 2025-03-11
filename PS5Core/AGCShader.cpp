/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "AGCShader.h"
#include "Assets.h"
#include "PS5MemoryAllocator.h"
#include <fstream>
#include <shader/shader_reflection.h>
#include "agc/error.h"

#include "../Assets/Shaders/PSSL/Interop.h"
#include "../Assets/Shaders/PSSL/ShaderConstants.psslh"

using namespace NCL;
using namespace PS5;

AGCShader::AGCShader(const std::string& filename, MemoryAllocator& allocator) {

	char* fileData = nullptr;
	size_t readSize = 0;

	if (!Assets::ReadBinaryFile(Assets::SHADERDIR + "PSSL/" + filename, &fileData, readSize)) {
		assert(false && "Failed to read shader file");
	}
	//Now to allocate some GPU-visible memory for the file data to sit in...

	allocation = (uint8_t*)allocator.Allocate(readSize, sizeof(uint64_t) );
	memcpy(allocation, fileData, readSize);

	SceShaderBinaryHandle binaryHandle = sceShaderGetBinaryHandle(allocation);
	const void* header = sceShaderGetProgramHeader(binaryHandle);
	const void* program = sceShaderGetProgram(binaryHandle);

	SceError error = sce::Agc::createShader(&binary, const_cast<void*>(header), program);

#ifndef _NDEBUG
	// Check that ShaderConstants is the same size in CPP and HLSL
	auto meta = sceShaderGetMetadataSection(binaryHandle);

	auto resourceList = sceShaderGetResourceList(meta);
	for (auto resource = sceShaderGetFirstResource(meta, resourceList); resource != nullptr; resource = sceShaderGetNextResource(meta, resource)) {
		auto type = sceShaderGetResourceClass(meta, resource);
		std::string_view name(sceShaderGetResourceName(meta, resource));

		if (type == SceShaderResourceClass::SceShaderCb && name == "constants") {
			auto type = sceShaderGetResourceType(meta, resource);
			auto size = sceShaderGetStructSize(meta, type);
				
			assert(size == sizeof(ShaderConstants));
		}
	}
#endif // !_NDEBUG
	delete fileData;
//	return shader;
}

void AGCShader::ReloadShader() {


}