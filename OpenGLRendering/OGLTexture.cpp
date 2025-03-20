/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#include "OGLTexture.h"
#include "OGLRenderer.h"

#include "TextureLoader.h"

using namespace NCL;
using namespace NCL::Rendering;

OGLTexture::OGLTexture()	{
	glGenTextures(1, &texID);
}

OGLTexture::OGLTexture(GLuint texToOwn) {
	texID = texToOwn;
}

OGLTexture::OGLTexture(const void* data, int width, int height, int channels)
{
    // set the unpack alignment to 1 to avoid any byte alignment issues
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction (default is 4 but we're using single byte per pixel)

    dimensions = Vector2ui(width, height);

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = (channels == 1) ? GL_RED : GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

OGLTexture::~OGLTexture()	{
	glDeleteTextures(1, &texID);
	free(texData);
}

UniqueOGLTexture OGLTexture::TextureFromData(char* data, uint32_t width, uint32_t height, uint32_t channels) {
	UniqueOGLTexture tex = std::make_unique<OGLTexture>();
	tex->texData = data;
	tex->width = width;
	tex->height = height;
	tex->channels = channels;
	tex->upload();
	return tex;
}

UniqueOGLTexture OGLTexture::TextureFromFile(const std::string&name) {
	UniqueOGLTexture tex = std::make_unique<OGLTexture>();
	tex->load(name);
	tex->upload();
	tex->fileName = name;
	return tex;
}

UniqueOGLTexture OGLTexture::LoadCubemap(
	const std::string& xPosFile,
	const std::string& xNegFile,
	const std::string& yPosFile,
	const std::string& yNegFile,
	const std::string& zPosFile,
	const std::string& zNegFile) {

	const std::string* filenames[6] = {&xPosFile,&xNegFile,&yPosFile,&yNegFile,&zPosFile,&zNegFile};
	uint32_t width[6] = { 0 };
	uint32_t height[6] = { 0 };
	uint32_t channels[6] = { 0 };
	uint32_t flags[6] = { 0 };

	std::vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(*filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return nullptr;
		}
	}

	UniqueOGLTexture tex = std::make_unique<OGLTexture>();
	tex->dimensions = { width[0], height[0] };

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex->GetObjectID());

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return tex;
}

void OGLTexture::load(const std::string& name)
{
	TextureLoader::LoadTexture(name, texData, width, height, channels, flags);
}

void OGLTexture::upload()
{
	int dataSize = width * height * channels; //This always assumes data is 1 byte per channel

	int sourceType = GL_RGB;

	switch (channels) {
		case 1: sourceType = GL_RED	; break;
		case 2: sourceType = GL_RG	; break;
		case 3: sourceType = GL_RGB	; break;
		case 4: sourceType = GL_RGBA; break;
	}

	glBindTexture(GL_TEXTURE_2D, texID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, sourceType, GL_UNSIGNED_BYTE, texData);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	free(texData);
	texData = nullptr;
}
