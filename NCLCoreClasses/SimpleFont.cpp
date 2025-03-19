#include "SimpleFont.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Assets.h"

#include <ft2build.h>
#include FT_FREETYPE_H

using namespace NCL;
using namespace Rendering;
using namespace Maths;

SimpleFont::SimpleFont(const std::string&filename) {
    ft = nullptr; // Initialize the pointer
    InitializeFreeType(filename);
}


SimpleFont::~SimpleFont()	{
    characters.clear(); // Explicitly clear the map, good practice

    if (ft) {
        FT_Done_FreeType(reinterpret_cast<FT_Library>(ft)); // Cast back to FreeType and free it
    }
}

int SimpleFont::InitializeFreeType(const std::string& filename) {
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

    // Setting FT_LOAD_RENDER flag to render the character as an 8-bit grayscale bitmap
    // This is required to extract the glyph's bitmap data and can be accessed through face->glyph->bitmap
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) { // Load character X
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        return -1;
    }

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Create Texture from glyph bitmap
        SharedTexture texture = std::make_shared<Texture>(
            face->glyph->bitmap.buffer, 
            face->glyph->bitmap.width, 
            face->glyph->bitmap.rows, 
            1 // single channel
        );

        // now store character for later use
        Character character = {
            texture,
            Vector2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            Vector2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x >> 6 // bitshift by 6 to get value in pixels (2^6 = 64)
        };

        characters.insert(std::pair<char, Character>(c, character));
    }

    // clear the face and free the FreeType library
    FT_Done_Face(face);
    return 0;
}