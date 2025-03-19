/******************************************************************************
This file is part of the Newcastle PS5 Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Texture.h"

#include <agc.h>

namespace NCL::PS5 {
	class MemoryAllocator;

	class AGCTexture : public NCL::Rendering::Texture {
	public:
		AGCTexture(const std::string& filename, const MemoryAllocator& a);
		AGCTexture(const MemoryAllocator& a) { allocator = &a; }
		AGCTexture();
		~AGCTexture();

		void setAllocator(const MemoryAllocator* alloc) { allocator = alloc; }

		const sce::Agc::Core::Texture* GetAGCPointer() const {
			return &agcTex;
		}
		sce::Agc::Core::Texture* GetAGCPointer() {
			return &agcTex;
		}

		// This also uploads on PS5, safe due to its unified memory architecture
		void load(const std::string& name) override;
		void upload() override {};
	protected:


	private:
		const MemoryAllocator* allocator = nullptr;
		void* gpuAllocation;
		sce::Agc::Core::Texture agcTex;
	};
}