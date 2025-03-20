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
            struct GlyphData {
                Vector2ui size;
                Vector2 bearing;
                int advance;
                std::vector<char> data;
            };

            // Metrics of a character glyph
            struct Character {
                Vector2 size;      // Size of glyph
                Vector2   bearing;   // Offset from baseline to left/top of glyph
                Vector2 uvTopLeft;
                Vector2 uvBottomRight;
                unsigned int advance;   // Offset to advance to next glyph
            };
#
            // Texture will be filled with the font's glyphs
			SimpleFont(const std::string&fontName, std::shared_ptr<Texture> texture);
			~SimpleFont();

            int InitializeFreeType(const std::string& filename, std::shared_ptr<Texture> texture);

            Character GetCharacter(unsigned char c) const {
                return characters.at(c);
            }

            std::shared_ptr<Texture> getTexture() { return texture; }

		protected:
            const static constexpr int StartChar = 0;
            const static constexpr int EndChar = 128;
            const static constexpr int AtlasSize = 12;
            static_assert(StartChar == 0, "Not tested");
            static_assert((AtlasSize* AtlasSize) > (StartChar - EndChar), "Atlas too small for characters");


            std::shared_ptr<Texture> texture;
            std::map<unsigned char, Character> characters;
		};
	}
}

