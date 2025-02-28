#include "DecalSystem.h"
#include "ResourceManager.h"
#include <iostream>

using namespace NCL::Rendering;
using namespace NCL::CSC8503;

DecalSystem::DecalSystem(int width, int height) 
	: decayRate(0.1f), textureWidth(width), textureHeight(height), alphaFade(1.0f)
{
	// Create an empty texture to store the applied decals
	glGenTextures(1, &decalTexture);
	glBindTexture(GL_TEXTURE_2D, decalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create a new FBO to render the decals to
	glGenFramebuffers(1, &decalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, decalTexture, 0);

	// Check if FBO is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Decal FBO is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind FBO
}

DecalSystem::~DecalSystem()
{
	glDeleteTextures(1, &decalTexture);
	glDeleteFramebuffers(1, &decalFBO);
}

void DecalSystem::ApplyDecal(Decal& decal)
{
	// Add a new decal to the list
	std::cout << "Decal Texture" << decal.texture << std::endl;
	decals.emplace_back(decal);
	alphaFade = 1.0f; // Reset alpha fade when adding a new decal
}

void DecalSystem::Update(float dt)
{
	// Reduce the alpha fade gradually over time
	alphaFade = std::max(alphaFade - (decayRate * dt), 0.0f);
}
