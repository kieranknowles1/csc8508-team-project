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
        data.advance = face->glyph->advance.x * 64;
        data.bearing = { float(face->glyph->bitmap_left), float(face->glyph->bitmap_top) };
        data.size = {
            face->glyph->bitmap.width, face->glyph->bitmap.rows
        };
        data.data.resize(data.size.x * data.size.y);
        std::memcpy(data.data.data(), face->glyph->bitmap.buffer, data.size.x * data.size.y);

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
        ch.size = Vector2(glyph.size.x, glyph.size.y);
        ch.bearing = glyph.bearing;
        ch.uvTopLeft = Vector2((1.0f / AtlasSize) * row, (1.0f / AtlasSize) * col);
        ch.uvBottomRight = ch.uvTopLeft * (pixelSize * ch.size);
        ch.advance = glyph.advance;
        characters[i] = ch;
    }

    texture->upload();

    // clear the face and free the FreeType library
    FT_Done_Face(face);
    FT_Done_FreeType(lib); // Cast back to FreeType and free it
    return 0;
}