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

    // `reserve` is used to pre-allocate memory for the vectors to avoid reallocations
    positions.reserve(positions.size() + (text.length() * 6));
    colours.reserve(colours.size() + (text.length() * 6));
    texCoords.reserve(texCoords.size() + (text.length() * 6));

    // Iterate through each character in the string
    for (auto ch : text) {
        // Handle newlines
        if (ch == '\n') {
            currentX = startPos.x;
            currentY += 0.1f * size;
        }
        // Skip carriage returns
        // Ignore windows line endings, LF covers both Windows and Unix
        if (ch == '\r') continue;

        // Find the character glyph data in the map
        auto data = characters.find(ch);
        if (data == characters.end()) continue;
        auto& info = data->second;

        // Calculate the positions of the four corners of the character quad
        Vector3 topLeft = Vector3(currentX, currentY, 0) * size;
        topLeft.x += info.bearing.x * size;
        topLeft.y -= info.bearing.y * size;

        Vector3 bottomRight = topLeft + Vector3(info.size, 0) * size;
        Vector3 bottomLeft(topLeft.x, bottomRight.y, 0);
        Vector3 topRight(bottomRight.x, topLeft.y, 0);

        // Calculate the UVs (screen-space positions) for the character quad
        Vector2 uvBottomLeft(info.uvTopLeft.x, info.uvBottomRight.y);
        Vector2 uvTopRight(info.uvBottomRight.x, info.uvTopLeft.y);

        for (int i = 0; i < 6; i++) {
            colours.push_back(colour);
        }

        // OpenGL uses counter-clockwise winding
        // topLeft -> bottomLeft -> topRight
        // topRight -> bottomRight -> bottomLeft

        // Store vertex data for the quad into the buffers
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

        // Moves the cursor to the right based on the character's width/advance
        currentX += info.advance * advanceMult;
    }
}

void SimpleFont::BuildInterleavedVerticesForString(const std::string& text, const Maths::Vector2& startPos, const Maths::Vector4& colour, float size, std::vector<InterleavedTextVertex>& vertices)
{
    // Rich's code duplicated all of this, so consider this version less bad
    // Create the vertex data for the string
    std::vector<Vector3> positions;
    std::vector<Vector4> colours;
    std::vector<Vector2> uvs;

    // Calls BuildVerticesForString to generate the vertex data to fill the vectors (be interleaved)
    // based on the input text
    BuildVerticesForString(text, startPos, colour, size, positions, uvs, colours);

    // Asserts that all vectors have the same size to ensure data consistency
    assert(positions.size() == colours.size());
    assert(positions.size() == uvs.size());

    // Loops through the filled vectors, creating InterleavedTextVertex objects/structs
    for (int i = 0; i < positions.size(); i++) {
        InterleavedTextVertex vtx;
        vtx.colour = colours[i];
        vtx.pos = Vector2(positions[i].x, positions[i].y);
        vtx.texCoord = uvs[i];
        vertices.push_back(vtx);
    }
}

SimpleFont::SimpleFont(const std::string&filename, std::shared_ptr<Texture> texture, float advanceMult)
    : texture(texture)
    , advanceMult(advanceMult)
{
    InitializeFreeType(filename, texture);
}


SimpleFont::~SimpleFont()	{
}

// Get the next power of 2 that is >= `i`
int nextPower2(int i) {
    int candidate = 1;
    while (candidate < i) {
        candidate = candidate * 2;
    }
    return candidate;
}

// Convert a 26.6 fixed point integer into a float
float fixedToFloat(int fixed) {
    return fixed / 64.0f;
}

int SimpleFont::InitializeFreeType(const std::string& filename, std::shared_ptr<Texture> texture) {
    // Initialize FreeType library
    FT_Library lib;
    if (FT_Init_FreeType(&lib)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }

    // Load a font face from a file
    FT_Face face;
    if (FT_New_Face(lib, (Assets::FONTSSDIR + filename).c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        FT_Done_FreeType(lib); // Free library before returning (not needed but good practice for resource management)
        return -1;
    }

    // Sets the size of the glyph in pixels
    FT_Set_Pixel_Sizes(face, 0, 48); // 0 width lets FreeType calculate the width based on the height

    std::vector<GlyphData> glyphs;
    Vector2ui maxSize(0, 0);

    // Setting FT_LOAD_RENDER flag to render the character as an 8-bit grayscale bitmap
    // This is required to extract the glyph's bitmap data and can be accessed through face->glyph->bitmap
    
    // Loop through all ASCII characters and extract the glyphs
    for (unsigned char c = StartChar; c < EndChar; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Store the glyph data
        GlyphData data;
        data.ch = c;
        data.advance = fixedToFloat(face->glyph->advance.x);
        data.bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };

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
    // PS5 textures are very sensitive to alignment, rounding sizes up to a power of 2 seems safer
    texture->width = nextPower2(maxSize.x * AtlasSize);
    texture->height = nextPower2(maxSize.y * AtlasSize);
    texture->channels = 1;
    size_t bytes = texture->width * texture->height;
    texture->texData = (char*)malloc(bytes);
    memset(texture->texData, 0, bytes);

    Vector2 pixelSize(1.0f / texture->width, 1.0f / texture->height);
    // Loop through all the glyphs and copy the data into the texture
    for (int i = 0; i < glyphs.size(); i++) {
        // Get the glyph data
        auto& glyph = glyphs[i];
        int row = int(i / AtlasSize);
        int col = i % AtlasSize;
        char* dataBegin = texture->texData + (row * texture->width * maxSize.y) + (col * maxSize.x);

        // Copy the glyph data into the texture
        for (int y = 0; y < glyph.size.y; y++) {
            memcpy(dataBegin + (y * texture->width), glyph.data.data() + (y * glyph.size.x), glyph.size.x);
        }

        // Store the character data
        // Keep everything in screen multiples, for easier use with NDCs
        Character ch;
        ch.size = Vector2(glyph.size.x, glyph.size.y) / ScreenSize;
        ch.bearing = Vector2(glyph.bearing.x, glyph.bearing.y) / ScreenSize;
        ch.advance = glyph.advance / ScreenSize.x;

        // Calculate the UVs for the character
        int pixelsLeft = col * maxSize.x;
        int pixelsTop = row * maxSize.y;
        ch.uvTopLeft = Vector2(pixelsLeft, pixelsTop) * pixelSize;
        ch.uvBottomRight = ch.uvTopLeft + (pixelSize * Vector2(glyph.size.x, glyph.size.y));

        // Store the character in the map
        characters[glyph.ch] = ch;
    }

    // Upload the texture to the GPU
    texture->upload();

    // clear the face and free the FreeType library
    FT_Done_Face(face);
    FT_Done_FreeType(lib); // Cast back to FreeType and free it
    return 0;
}
