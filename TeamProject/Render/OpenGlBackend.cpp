#include "OpenGlBackend.h"

#include <NCLCoreClasses/MshLoader.h>

#include <OpenGLRendering/OGLMesh.h>
#include <OpenGLRendering/OGLTexture.h>
#include <OpenGLRendering/OGLShader.h>

namespace NCL::CSC8503::Render {
	OpenGlBackend::OpenGlBackend(Window* window)
		: OGLRenderer(*window)
	{
	}

	void OpenGlBackend::beginFrame()
	{
		BeginFrame();
	}

	void OpenGlBackend::swapBuffers()
	{
		EndFrame();
		SwapBuffers();
	}

	Rendering::Mesh* OpenGlBackend::loadMesh(std::string_view name)
	{
		OGLMesh* mesh = new OGLMesh();
		MshLoader::LoadMesh(std::string(name), *mesh);
		mesh->SetPrimitiveType(GeometryPrimitive::Triangles);
		mesh->UploadToGPU();
		return mesh;
	}
	Rendering::Texture* OpenGlBackend::loadTexture(std::string_view name)
	{
		return OGLTexture::TextureFromFile(std::string(name)).release();
	}
	Rendering::Shader* OpenGlBackend::loadShader(std::string_view vertex, std::string_view fragment)
	{
		return new OGLShader(std::string(vertex), std::string(fragment));
	}

	// TOOD: Make these functions lazy, don't bind anything if it is already bound
	void OpenGlBackend::useShader(Rendering::Shader* shader)
	{
		UseShader(*(OGLShader*)shader);
	}
	void OpenGlBackend::bindMesh(Rendering::Mesh* mesh)
	{
		BindMesh(*(OGLMesh*)mesh);
	}
	void OpenGlBackend::drawBoundMesh(int subLayer, int numInstances)
	{
		DrawBoundMesh(subLayer, numInstances);
	}
	void OpenGlBackend::bindTextureToShader(const Rendering::Texture* t, const std::string& uniform, int texUnit)
	{
		BindTextureToShader(*(OGLTexture*)t, uniform, texUnit);
	}
}