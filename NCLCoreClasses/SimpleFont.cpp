#include "SimpleFont.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Assets.h"

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace NCL;
using namespace Rendering;
using namespace Maths;

void SimpleFont::BuildVerticesForString(const std::string& text, const Maths::Vector2& startPos, const Maths::Vector4& colour, float size, std::vector<Maths::Vector3>& positions, std::vector<Maths::Vector2>& texCoords, std::vector<Maths::Vector4>& colours)
{
    float currentX = startPos.x;
    float currentY = startPos.y;

    positions.reserve(positions.size() + (text.length() * 6));
    colours.reserve(colours.size() + (text.length() * 6));
    texCoords.reserve(texCoords.size() + (text.length() * 6));

    for (auto ch : text) {
        if (ch == '\n') {
            currentX = startPos.x;
            currentY += 0.1f * size;
        }
        if (ch == '\r') continue;
        auto data = characters.find(ch);
        if (data == characters.end()) continue;
        auto& info = data->second;

        Vector3 topLeft = Vector3(currentX, currentY, 0) * size;
        Vector3 bottomRight = topLeft + Vector3(info.size, 0) * size;
        Vector3 bottomLeft(topLeft.x, bottomRight.y, 0);
        Vector3 topRight(bottomRight.x, topLeft.y, 0);

        Vector2 uvBottomLeft(info.uvTopLeft.x, info.uvBottomRight.y);
        Vector2 uvTopRight(info.uvBottomRight.x, info.uvTopLeft.y);

        for (int i = 0; i < 6; i++) {
            colours.push_back(colour);
        }

        // OpenGL uses counter-clockwise winding
        // topLeft -> bottomLeft -> topRight
        // topRight -> bottomRight -> bottomLeft

        positions.push_back(topLeft);
        texCoords.push_back(info.uvTopLeft);
        positions.push_back(bottomLeft);
        texCoords.push_back(uvBottomLeft);
        positions.push_back(topRight);
        texCoords.push_back(uvTopRight);

        positions.push_back(topRight);
        texCoords.push_back(uvTopRight);
        positions.push_back(bottomRight);
        texCoords.push_back(info.uvBottomRight);
        positions.push_back(bottomLeft);
        texCoords.push_back(uvBottomLeft);

        // TODO: Handle advance properly
        //currentX += (topRight.x - topLeft.x);
        currentX += (info.advance / 64) / ScreenSize.x;
    }
}

SimpleFont::SimpleFont(const std::string&filename, std::shared_ptr<Texture> texture) {
    this->texture = texture;
    InitializeFreeType(filename, texture);
}


SimpleFont::~SimpleFont()	{
}

int SimpleFont::InitializeFreeType(const std::string& filename, std::shared_ptr<Texture> texture) {
    FT_Library lib;
    if (FT_Init_FreeType(&lib)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(lib, (Assets::FONTSSDIR + filename).c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        FT_Done_FreeType(lib); // Free library before returning (not needed but good practice for resource management)
        return -1;
    }

    FT_Set_Pixel_Sizes(face, 0, 48); // 0 width lets FreeType calculate the width based on the height

    std::vector<GlyphData> glyphs;
    Vector2ui maxSize(0, 0);

    // Setting FT_LOAD_RENDER flag to render the character as an 8-bit grayscale bitmap
    // This is required to extract the glyph's bitmap data and can be accessed through face->glyph->bitmap
    for (unsigned char c = StartChar; c < EndChar; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }
        GlyphData data;
        data.ch = c;
        data.advance = face->glyph->advance.x;
        data.bearing = { float(face->glyph->bitmap_left), float(face->glyph->bitmap_top) };

        data.size = {
            face->glyph->bitmap.width, face->glyph->bitmap.rows
        };
        size_t bytes = data.size.x * data.size.y;
        // https://en.cppreference.com/w/c/string/byte/memcpy
        // data or glyph.bitmap.buffer could be nullptr for an empty
        // glyph, which is undefined behaviour even if size is 0
        if (bytes > 0) {
            data.data.resize(bytes);
            memcpy(data.data.data(), face->glyph->bitmap.buffer, bytes);
        }

        maxSize.x = std::max(maxSize.x, data.size.x);
        maxSize.y = std::max(maxSize.y, data.size.y);
        glyphs.emplace_back(std::move(data));
    }

    // Create an atlas for our texture
    texture->width = maxSize.x * AtlasSize;
    texture->height = maxSize.y * AtlasSize;
    texture->channels = 1;
    size_t bytes = texture->width * texture->height;
    texture->texData = (char*)malloc(bytes);
    memset(texture->texData, 0, bytes);

    Vector2 pixelSize(1.0f / texture->width, 1.0f / texture->height);
    for (int i = 0; i < glyphs.size(); i++) {
        auto& glyph = glyphs[i];
        int row = int(i / AtlasSize);
        int col = i % AtlasSize;
        char* dataBegin = texture->texData + (row * texture->width * maxSize.y) + (col * maxSize.x);

        for (int y = 0; y < glyph.size.y; y++) {
            memcpy(dataBegin + (y * texture->width), glyph.data.data() + (y * glyph.size.x), glyph.size.x);
        }

        Character ch;
        Vector2 sizeVec = Vector2(glyph.size.x, glyph.size.y);
        ch.size = sizeVec / ScreenSize;
        ch.bearing = glyph.bearing;
        ch.uvTopLeft = Vector2((1.0f / AtlasSize) * col, (1.0f / AtlasSize) * row);
        ch.uvBottomRight = ch.uvTopLeft + (pixelSize * sizeVec);
        ch.advance = glyph.advance;
        characters[glyph.ch] = ch;
    }

    texture->upload();

    // clear the face and free the FreeType library
    FT_Done_Face(face);
    FT_Done_FreeType(lib); // Cast back to FreeType and free it
    return 0;
}
