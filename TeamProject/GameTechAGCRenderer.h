#pragma once

#include <mutex>

#include "GameTechRendererInterface.h"

#include "../PS5Core/AGCRenderer.h"

#include "../PS5Core/AGCBuffer.h"

#include "./Shaders/PSSL/ShaderConstants.psslh"
#include "Shaders/PSSL/types.h"

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

			RendererBase* getBase() override {
				return this;
			}

			virtual Mesh*		LoadMesh(const std::string& name)				override;
			virtual Texture*	LoadTexture(const std::string& name)			override;

			void RegisterTexture(const std::string& name, PS5::AGCTexture* outTex);

		protected:
			std::mutex texMapMtx;

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

			void MainRenderPass();

			void UiPass();
			void LightPass();
			void PostProcessPass();

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
				template <typename T>
				struct UniformArray {
					sce::Agc::Core::Buffer buffer;
					char* start;

					void begin(FrameData* frame);
					void end(FrameData* frame);
				};

				UniformArray<ObjectState> objects;
				UniformArray<UiState> ui;
				UniformArray<LightState> lights;

				sce::Agc::Core::Buffer constantBuffer;

				sce::Agc::Core::Buffer debugLineBuffer;
				sce::Agc::Core::Buffer debugTextBuffer;

				BumpAllocator data;

				int globalDataOffset	= 0;	//Where does the global data start in the buffer?
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
			PS5::AGCMesh* sphere;

			sce::Agc::Core::Texture*	bindlessTextures;
			sce::Agc::Core::Buffer*		bindlessBuffers;
			uint32_t bufferCount;

			sce::Agc::Core::Buffer textureBuffer;
			std::map<std::string, PS5::AGCTexture*> textureMap;

			sce::Agc::Core::Buffer arrayBuffer;

			std::unique_ptr<PS5::AGCShader> skinningCompute;

			std::unique_ptr<PS5::AGCShader> defaultVertexShader;
			std::unique_ptr<PS5::AGCShader> defaultPixelShader;

			std::unique_ptr<PS5::AGCShader> uiVertexShader;
			std::unique_ptr<PS5::AGCShader> uiPixelShader;

			std::unique_ptr<PS5::AGCShader> debugLineVertexShader;
			std::unique_ptr<PS5::AGCShader> debugLinePixelShader;

			std::unique_ptr<PS5::AGCShader> debugTextVertexShader;
			std::unique_ptr<PS5::AGCShader> debugTextPixelShader;

			std::unique_ptr<PS5::AGCShader> gammaCompute;

			std::unique_ptr<PS5::AGCShader> deferredVertexShader;
			std::unique_ptr<PS5::AGCShader> deferredPixelShader;

			std::unique_ptr<PS5::AGCShader> postVertexShader;
			std::unique_ptr<PS5::AGCShader> postPixelShader;

			struct FrameBuffer {
				enum class Slot {
					Color,
					Normal,
					Specular = Normal,
				};

				sce::Agc::CxRenderTarget target;
				PS5::AGCTexture* texture;
				sce::Agc::Core::Sampler sampler;
			};

			FrameBuffer createBuffer(const std::string& name, FrameBuffer::Slot slot);

			// Configure a viewport that takes up the requested size
			void useViewPort(sce::Agc::Core::BasicContext* context, Vector2i size) const;

			FrameBuffer sceneBuffer;
			FrameBuffer sceneNormalBuffer;

			FrameBuffer lightDiffuse;
			FrameBuffer lightSpecular;

			FrameBuffer screenBuffer;

			std::vector<SkinningJob> frameJobs;
		};
}
}
