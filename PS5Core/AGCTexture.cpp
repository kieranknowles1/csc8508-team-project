/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "AGCTexture.h"
#include "PS5MemoryAllocator.h"
#include "Assets.h"

#include <fstream>
#include <filesystem>

using namespace NCL;
using namespace PS5;

AGCTexture::AGCTexture(const std::string& filename, const MemoryAllocator& a)
{
	allocator = &a;
	load(filename);
}

AGCTexture::AGCTexture()
{
	gpuAllocation = 0;

}

AGCTexture::~AGCTexture() {

}

void AGCTexture::load(const std::string& filename)
{
	std::filesystem::path path(filename + ".gnf");
	this->fileName = filename;
	//path.replace_extension(".gnf");
	std::string realFile = Assets::TEXTUREDIR + path.string();
	FILE* fp = fopen(realFile.c_str(), "rb");

	if (!fp) {
		std::cout << __FUNCTION__ << "Can't load file " << filename << "\n";
		return;
	}

	//SCE_AGC_ASSERT_MSG_RETURN(fp, SCE_AGC_ERROR_FAILURE, "Could not open file %s", filename);
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	if (sz <= 0)
	{
		SCE_AGC_ASSERT_MSG("Could not get the size of the file %s", filename);
		fclose(fp);
		//return SCE_AGC_ERROR_FAILURE;
	}
	fseek(fp, 0, SEEK_SET);


	gpuAllocation = allocator->Allocate(sz, 64 * 1024);
	printf("%s(%s): p=%p, sz=%lu\n", __func__, filename.c_str(), gpuAllocation, sz);
	size_t bytesRead = fread(gpuAllocation, 1, sz, fp);
	if (bytesRead != sz)
	{
		SCE_AGC_ASSERT_MSG("Could not read the entire contents of the file %s", filename);
		fclose(fp);
		//return SCE_AGC_ERROR_FAILURE;
	}
	fclose(fp);

	SceError err = sce::Agc::Core::translate(&agcTex, (sce::Gnf::GnfFileV5*)gpuAllocation);
	//if (err == SCE_OK)
	//{
	//	sce::Agc::Core::registerResource(&agcTex, "%s", filename.c_str());
	//}
}

void AGCTexture::upload(bool freeData)
{
	if (texData == nullptr) return;
	assert(channels == 1 && "Multiple channels are not supported.");

	size_t sz = width * height * sizeof(float);
	gpuAllocation = allocator->Allocate(sz, 64 * 1024);

	// HACK: Manually convert the uint8 data to float32
	// Core::translate could probably do this, but there's over 100 overloads to look through
	unsigned char* ptr = (unsigned char*)texData;
	float* gpuPtr = (float*)gpuAllocation;
	for (int i = 0; i < width * height; i++) {
		*gpuPtr = float(*ptr / 256.0f);
		gpuPtr++;
		ptr++;
	}

	agcTex.init()
		.setType(sce::Agc::Core::Texture::Type::k2d)
		.setSwizzle(sce::Agc::Core::Swizzle::kR000_S1) // 1 channel
		.setFormat(sce::Agc::Core::TypedFormat::k32Float) // 1 float
		.setNumMipLevels(1)
		.setWidth(width).setHeight(height)
		.setDataAddress(gpuAllocation);

	//agcTex.
}
