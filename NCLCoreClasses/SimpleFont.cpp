/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#include "SimpleFont.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Assets.h"

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace NCL;
using namespace Rendering;
using namespace Maths;

SimpleFont::SimpleFont(const std::string&filename, Texture& tex) : texture(tex)	{
	startChar	= 0;
	numChars	= 0;
	allCharData	= nullptr;

	std::ifstream fontFile(Assets::FONTSSDIR + filename);

	fontFile >> texWidth;
	fontFile >> texHeight;
	fontFile >> startChar;
	fontFile >> numChars;

	allCharData = new FontChar[numChars];

	for (int i = 0; i < numChars; ++i) {
		fontFile >> allCharData[i].x0;
		fontFile >> allCharData[i].y0;
		fontFile >> allCharData[i].x1;
		fontFile >> allCharData[i].y1;

		fontFile >> allCharData[i].xOff;
		fontFile >> allCharData[i].yOff;
		fontFile >> allCharData[i].xAdvance;
	}
	texWidthRecip	= 1.0f / texWidth;
	texHeightRecip	= 1.0f / texHeight;
    
    InitializeFreeType(filename);
}


SimpleFont::~SimpleFont()	{
	delete[]	allCharData;
}

int SimpleFont::InitializeFreeType(const std::string& filename) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, (Assets::FONTSSDIR + filename).c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
}

int SimpleFont::GetVertexCountForString(const std::string& text) {
	int count = 0;
	for (auto ch : text) {
		if (ch != '\n' && ch != '\r') {
			count += 6;
		}
	}
	return count;
}

void SimpleFont::BuildVerticesForString(const std::string& text, const Vector2& startPos, const Vector4& colour, float size, std::vector<Vector3>& positions, std::vector<Vector2>& texCoords, std::vector<Vector4>& colours) {
	int endChar = startChar + numChars;

	float currentX = 0.0f;
	float currentY = startPos.y;

	positions.reserve(positions.size() + (text.length() * 6));
	colours.reserve(colours.size() + (text.length() * 6));
	texCoords.reserve(texCoords.size() + (text.length() * 6));

	for (char charIndex : text) {
		if (charIndex == '\n') {
			currentX = 0;
			currentY += 0.25f * size;
			continue;
		}
		if (charIndex == '\r') {
			continue;
		}
		if (charIndex < startChar) {
			continue;
		}
		if (charIndex > endChar) {
			continue;
		}
		FontChar& charData = allCharData[charIndex - startChar];

		float scale = size;
		//For basic vertex buffers, we're assuming we should add 6 vertices

		float charWidth = (float)((charData.x1 - charData.x0) / texWidth) * scale;
		float charHeight = (float)(charData.y1 - charData.y0);

		float xStart = ((charData.xOff + currentX) * texWidthRecip) * scale;
		float yStart = currentY;
		float yHeight = (charHeight * texHeightRecip) * scale;
		float yOff = ((charHeight + charData.yOff) * texHeightRecip) * scale;

		positions.emplace_back(Vector3(startPos.x + xStart, yStart + yOff, 0));
		positions.emplace_back(Vector3(startPos.x + xStart, yStart + yOff - yHeight, 0));
		positions.emplace_back(Vector3(startPos.x + xStart + charWidth, yStart + yOff - yHeight, 0));

		positions.emplace_back(Vector3(startPos.x + xStart + charWidth, yStart + yOff - yHeight, 0));
		positions.emplace_back(Vector3(startPos.x + xStart + charWidth, yStart + yOff, 0));
		positions.emplace_back(Vector3(startPos.x + xStart, yStart + yOff, 0));

		colours.emplace_back(colour);
		colours.emplace_back(colour);
		colours.emplace_back(colour);

		colours.emplace_back(colour);
		colours.emplace_back(colour);
		colours.emplace_back(colour);

		texCoords.emplace_back(Vector2(charData.x0 * texWidthRecip, charData.y1 * texHeightRecip));
		texCoords.emplace_back(Vector2(charData.x0 * texWidthRecip, charData.y0 * texHeightRecip));
		texCoords.emplace_back(Vector2(charData.x1 * texWidthRecip, charData.y0 * texHeightRecip));

		texCoords.emplace_back(Vector2(charData.x1 * texWidthRecip, charData.y0 * texHeightRecip));
		texCoords.emplace_back(Vector2(charData.x1 * texWidthRecip, charData.y1 * texHeightRecip));
		texCoords.emplace_back(Vector2(charData.x0 * texWidthRecip, charData.y1 * texHeightRecip));

		currentX += charData.xAdvance;
	}
}

void SimpleFont::BuildInterleavedVerticesForString(const std::string& text, const Maths::Vector2& startPos, const Maths::Vector4& colour, float size, std::vector<InterleavedTextVertex>& vertices) {
	int endChar = startChar + numChars;

	float currentX = 0.0f;
	float currentY = startPos.y;

	vertices.reserve(text.length() * 6);

	InterleavedTextVertex verts[6];

	for (char charIndex : text) {
		if (charIndex == '\n') {
			currentX = 0;
			currentY += 0.25f * size;
			continue;
		}
		if (charIndex == '\r') {
			continue;
		}
		if (charIndex < startChar) {
			continue;
		}
		if (charIndex > endChar) {
			continue;
		}
		FontChar& charData = allCharData[charIndex - startChar];

		float scale = size;
		//For basic vertex buffers, we're assuming we should add 6 vertices

		float charWidth = (float)((charData.x1 - charData.x0) / texWidth) * scale;
		float charHeight = (float)(charData.y1 - charData.y0);

		float xStart = ((charData.xOff + currentX) * texWidthRecip) * scale;
		float yStart = currentY;
		float yHeight = (charHeight * texHeightRecip) * scale;
		float yOff = ((charHeight + charData.yOff) * texHeightRecip) * scale;

		verts[0].pos = Vector2(startPos.x + xStart, yStart + yOff);
		verts[1].pos = Vector2(startPos.x + xStart, yStart + yOff - yHeight);
		verts[2].pos = Vector2(startPos.x + xStart + charWidth, yStart + yOff - yHeight);

		verts[3].pos = (Vector2(startPos.x + xStart + charWidth, yStart + yOff - yHeight));
		verts[4].pos = Vector2(startPos.x + xStart + charWidth, yStart + yOff);
		verts[5].pos = Vector2(startPos.x + xStart, yStart + yOff);

		verts[0].colour = colour;
		verts[1].colour = colour;
		verts[2].colour = colour;

		verts[3].colour = colour;
		verts[4].colour = colour;
		verts[5].colour = colour;

		verts[0].texCoord = Vector2(charData.x0 * texWidthRecip, charData.y1 * texHeightRecip);
		verts[1].texCoord = Vector2(charData.x0 * texWidthRecip, charData.y0 * texHeightRecip);
		verts[2].texCoord = Vector2(charData.x1 * texWidthRecip, charData.y0 * texHeightRecip);

		verts[3].texCoord = Vector2(charData.x1 * texWidthRecip, charData.y0 * texHeightRecip);
		verts[4].texCoord = Vector2(charData.x1 * texWidthRecip, charData.y1 * texHeightRecip);
		verts[5].texCoord = Vector2(charData.x0 * texWidthRecip, charData.y1 * texHeightRecip);

		vertices.emplace_back(verts[0]);
		vertices.emplace_back(verts[1]);
		vertices.emplace_back(verts[2]);

		vertices.emplace_back(verts[3]);
		vertices.emplace_back(verts[4]);
		vertices.emplace_back(verts[5]);

		currentX += charData.xAdvance;
	}
}