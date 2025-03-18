/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once
#include "Vector.h"

namespace NCL {
	namespace Rendering {
		class Texture;

		class SimpleFont {
		public:
			SimpleFont(const std::string&fontName);
			~SimpleFont();

            int InitializeFreeType(const std::string& filename);

		protected:
            // Metrics of a character glyph
			struct Character {
                unsigned int textureID; // ID handle of the glyph texture
                Vector2 size;      // Size of glyph
                Vector2   bearing;   // Offset from baseline to left/top of glyph
                unsigned int advance;   // Offset to advance to next glyph
			};

            std::map<char, Character> characters;
		};
	}
}

