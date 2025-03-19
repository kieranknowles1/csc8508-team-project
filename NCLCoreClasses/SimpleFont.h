#pragma once
#include "Vector.h"
#include <string>
#include <map>

namespace NCL {
	namespace Rendering {
		class Texture;

		class SimpleFont {
		public:

            // Metrics of a character glyph
            struct Character {
                unsigned int textureID; // ID handle of the glyph texture
                Vector2 size;      // Size of glyph
                Vector2   bearing;   // Offset from baseline to left/top of glyph
                unsigned int advance;   // Offset to advance to next glyph
            };

			SimpleFont(const std::string&fontName);
			~SimpleFont();

            int InitializeFreeType(const std::string& filename);

		protected:
            void* ft; // Store as void* to avoid including FreeType headers
            std::map<unsigned char, Character> characters;
		};
	}
}

