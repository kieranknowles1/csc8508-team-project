#include "DecalSystem.h"
#include "ResourceManager.h"
#include <iostream>
#include <glad/gl.h>

using namespace NCL::Rendering;
using namespace NCL::CSC8503;

DecalSystem::DecalSystem(int width, int height)
    : decayRate(0.97f), textureWidth(width), textureHeight(height), gen(std::random_device{}()), angleDis(0.0f, 360.0f), indexDist(0, 0)
{
#ifndef __PROSPERO__
	// Create an empty texture to store the applied decals
	glGenTextures(1, &decalTexture);
	glBindTexture(GL_TEXTURE_2D, decalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);//GL_RGBA and GL_UNSIGNED_BYTE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create a new FBO to render the decals to
	glGenFramebuffers(1, &decalFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, decalTexture, 0);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, decalDepthTexture, 0);

	// Check if FBO is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Decal FBO is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind FBO
#endif
}

DecalSystem::~DecalSystem()
{
#ifndef __PROSPERO__
	glDeleteTextures(1, &decalTexture);
	glDeleteFramebuffers(1, &decalFBO);
#endif
}

void DecalSystem::ApplyDecal(Decal& decal)
{
	decals.emplace_back(decal);
}

void DecalSystem::Update(float dt)
{
	for (auto it = decals.begin(); it != decals.end();)
	{
		// Fade out the decal
		it->alphaFade -= (1- (it->alphaFade * decayRate )) * dt; // clamp alphaFade to 0.0f if it goes below 0.0f
		
		// Remove the decal if it has faded out
		if (it->alphaFade <= 0.0f) {
			it = decals.erase(it);
		}
		else {
			++it;
		}
	}
}

float DecalSystem::GetRandomRotation()
{
    return angleDis(gen);
}

std::shared_ptr<NCL::Rendering::Texture> DecalSystem::PickRandomDecal(const std::vector<std::shared_ptr<NCL::Rendering::Texture>>& decalsArr)
{
    if (decalsArr.empty()) {
        return nullptr;
    }

    indexDist = std::uniform_int_distribution<int>(0, decalsArr.size() - 1);

    return decalsArr[indexDist(gen)];
}