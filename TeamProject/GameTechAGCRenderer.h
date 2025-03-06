#pragma once

#include "GameTechRendererInterface.h"

#include "../PS5Core/AGCRenderer.h"

#include "../PS5Core/AGCBuffer.h"

#include "./Shaders/PSSL/Interop.h"				//Always include this before any PSSL headers
#include "./Shaders/PSSL/ShaderConstants.psslh"
#include "./Shaders/PSSL/TechObject.psslh"
#include "./Shaders/PSSL/UIObject.psslh"

namespace NCL {
	namespace Rendering {
		class Mesh;
		class Texture;
		class Shader;
	}
	namespace PS5 {
		class AGCTexture;
		class AGCShader;
	}

	namespace CSC8503 {
		class RenderObject;

		class GameTechAGCRenderer :
			public NCL::PS5::AGCRenderer,
			public NCL::CSC8503::GameTechRendererInterface
		{
		public:
			GameTechAGCRenderer(Window* window);
			~GameTechAGCRenderer();

			virtual Mesh*		LoadMesh(const std::string& name)				override;
			virtual Texture*	LoadTexture(const std::string& name)			override;

		protected:
			void checkError(SceError err) {
				assert(err == SCE_OK);
			}

			void drawFrame(float dt) override {
				Update(dt);
				Render();
			}

			void RenderFrame()	override;
			void UpdateObjectList();

			NCL::PS5::AGCTexture* CreateFrameBufferTextureSlot(const std::string& name);

			void WriteRenderPassConstants();
			void DrawObjects();
			void UpdateDebugData();

			void RenderDebugLines();
			void RenderDebugText();

			void ShadowmapPass();
			void SkyboxPass();
			void MainRenderPass();

			void UiPass();

			void DisplayRenderPass();

			Shader*		defaultShader;

			/*
			Handling buffers in AGC isn't too bad, as they are a small wrapper around an existing
			memory allocation. Here I have a small struct that will fill out a memory allocation with
			all of the data required by the frame. We can then make Buffers out of this at any
			offset we want to send to our shaders - in this case we're going to use one bug allocation
			to hold both the constants used by shaders, as well as all of the debug vertices, and object
			matrices. No fancy suballocations here, the allocator is as simple as it gets - it just
			advances or 'bumps' a pointer along. Perfect for recording a frame's data to memory!
			*/
			struct BumpAllocator {
				char* dataStart;//Start of our allocated memory
				char* data;		//Current write point of our memory
				size_t			bytesWritten;

				template<typename T>
				void WriteData(T value) {
					memcpy(data, &value, sizeof(T));
					data += sizeof(T);
					bytesWritten += sizeof(T);
				}
				void WriteData(void* inData, size_t byteCount) {
					memcpy(data, inData, byteCount);
					data += byteCount;
					bytesWritten += byteCount;
				}

				void AlignData(size_t alignment) {
					char* oldData = data;
					data = (char*)((((uintptr_t)data + alignment - 1) / (uintptr_t)alignment) * (uintptr_t)alignment);
					bytesWritten += data - oldData;
				}

				void Reset() {
					bytesWritten = 0;
					data = dataStart;
				}
			};

			struct FrameData {
				sce::Agc::Core::Buffer constantBuffer;
				sce::Agc::Core::Buffer objectBuffer;
				sce::Agc::Core::Buffer uiBuffer;

				sce::Agc::Core::Buffer debugLineBuffer;
				sce::Agc::Core::Buffer debugTextBuffer;

				BumpAllocator data;

				int globalDataOffset	= 0;	//Where does the global data start in the buffer?
				int objectStateOffset	= 0;	//Where does the object states start?
				int uiOffset = 0;
				int debugLinesOffset	= 0;	//Where do the debug lines start?
				int debugTextOffset		= 0;	//Where do the debug text verts start?

				size_t lineVertCount = 0;
				size_t textVertCount = 0;
			};

			struct SkinningJob {
				RenderObject* object;
				uint32_t outputIndex;
			};

			FrameData* allFrames;
			FrameData* currentFrame;
			int currentFrameIndex;

			std::unique_ptr<PS5::AGCMesh> unitQuad;
			std::unique_ptr<PS5::AGCMesh> halfUnitQuad;

			sce::Agc::Core::Texture*	bindlessTextures;
			sce::Agc::Core::Buffer*		bindlessBuffers;
			uint32_t bufferCount;

			sce::Agc::Core::Buffer textureBuffer;
			std::map<std::string, NCL::PS5::AGCTexture*> textureMap;

			sce::Agc::Core::Buffer arrayBuffer;

			NCL::PS5::AGCTexture* skyboxTexture;

			NCL::PS5::AGCShader* skinningCompute;

			NCL::PS5::AGCShader* defaultVertexShader;
			NCL::PS5::AGCShader* defaultPixelShader;

			std::unique_ptr<PS5::AGCShader> uiVertexShader;
			std::unique_ptr<PS5::AGCShader> uiPixelShader;

			NCL::PS5::AGCShader* shadowVertexShader;
			NCL::PS5::AGCShader* shadowPixelShader;

			NCL::PS5::AGCShader* skyboxVertexShader;
			NCL::PS5::AGCShader* skyboxPixelShader;

			NCL::PS5::AGCShader* debugLineVertexShader;
			NCL::PS5::AGCShader* debugLinePixelShader;

			NCL::PS5::AGCShader* debugTextVertexShader;
			NCL::PS5::AGCShader* debugTextPixelShader;

			NCL::PS5::AGCShader* gammaCompute;

			sce::Agc::CxDepthRenderTarget		shadowTarget;
			NCL::PS5::AGCTexture*				shadowMap; //ptr into bindless array
			sce::Agc::Core::Sampler				shadowSampler;

			void createBuffer(const std::string& name, sce::Agc::CxRenderTarget* outTarget, PS5::AGCTexture** outTexture, sce::Agc::Core::Sampler* optionalSampler);

			sce::Agc::CxRenderTarget			screenTarget;
			NCL::PS5::AGCTexture*				screenTex; //ptr into bindless array

			std::vector<SkinningJob> frameJobs;
		};
	}
}
