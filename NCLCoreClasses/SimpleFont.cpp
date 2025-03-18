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
#include "../OpenGLRendering/glad/gl.h"
#include FT_FREETYPE_H

using namespace NCL;
using namespace Rendering;
using namespace Maths;

SimpleFont::SimpleFont(const std::string&filename) {
    InitializeFreeType(filename);
}


SimpleFont::~SimpleFont()	{

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

    FT_Set_Pixel_Sizes(face, 0, 48); // 0 width lets FreeType calculate the width based on the height

    // Setting FT_LOAD_RENDER flag to render the character as an 8-bit grayscale bitmap
    // This is required to extract the glyph's bitmap data and can be accessed through face->glyph->bitmap
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) { // Load character X
        std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
        return -1;
    }

    // set the unpack alignment to 1 to avoid any byte alignment issues
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction (default is 4 but we're using single byte per pixel)

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED, // format of the texture, we're using a single channel for the bitmap
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // now store character for later use
        Character character = {
            texture,
            Vector2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            Vector2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };

        characters.insert(std::pair<char, Character>(c, character));
    }

    // clear the face and free the FreeType library
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}