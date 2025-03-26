#pragma once
#include "Vector.h"
#include <string>
#include <map>

using namespace NCL::Maths;

namespace NCL {
	namespace Rendering {
		class Texture;

		class SimpleFont {
		public:
            // A glyph that hasn't yet been inserted into an atlas
            // sizes are in pixels
            struct GlyphData {
                Vector2ui size; // Size of data
                Vector2i bearing; // Offset from baseline to top/left
                float advance; // Offset to reach the next glyph
                std::vector<char> data;
                char ch;
            };

            // A reference to a glyph in an atlas
            // sizes are in screens, (arbritray unit, but consistent across resolutions)
            // easier to work with in NDC space
            struct Character {
                Vector2 size; // Size of glyph
                Vector2   bearing;   // Offset from baseline to left/top of glyph
                Vector2 uvTopLeft;
                Vector2 uvBottomRight;
                float advance;   // Offset to advance to next glyph
            };


            struct InterleavedTextVertex {
                Maths::Vector2 pos;
                Maths::Vector2 texCoord;
                Maths::Vector4 colour;
            };

            void BuildVerticesForString(const std::string& text, const Maths::Vector2& startPos, const Maths::Vector4& colour, float size, std::vector<Maths::Vector3>& positions, std::vector<Maths::Vector2>& texCoords, std::vector<Maths::Vector4>& colours);
            void BuildInterleavedVerticesForString(const std::string& text, const Maths::Vector2& startPos, const Maths::Vector4& colour, float size, std::vector<InterleavedTextVertex>& vertices);

            // Texture will be filled with the font's glyphs
			SimpleFont(const std::string&fontName, std::shared_ptr<Texture> texture, float advanceMult = 1.0f);
			~SimpleFont();

            int InitializeFreeType(const std::string& filename, std::shared_ptr<Texture> texture);

            Character GetCharacter(unsigned char c) const {
                return characters.at(c);
            }

            std::shared_ptr<Texture> getTexture() { return texture; }

		protected:
            // Assume a constant size in pixels
            const static constexpr Vector2 ScreenSize = Vector2(1920, 1080);

            float advanceMult;

            // Start/end of ASCII printable characters
            const static constexpr int StartChar = 32;
            const static constexpr int EndChar = 126;
            const static constexpr int AtlasSize = 10;
            static_assert((AtlasSize* AtlasSize) > (StartChar - EndChar), "Atlas too small for characters");


            std::shared_ptr<Texture> texture;
            std::map<unsigned char, Character> characters;
		};
	}
}

