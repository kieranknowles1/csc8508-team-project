#pragma once

#include <cstdint>
#include<LinearMath/btVector3.h>

/**
 * Different Team Colors to choose from for each player.
 */
enum class TeamColor : uint32_t {
	RED = 0xFF0000FF,
	ORANGE = 0xFF8800FF,
	BLUE = 0x0000FFFF,
	GREEN = 0x00FF00FF,
	PURPLE = 0xFF00FFFF,
	PINK = 0xF24E88FF,
	YELLOW = 0xFFFF00FF,
	CYAN = 0x00FFFFFF
};

constexpr TeamColor colors[8] = {
	TeamColor::RED,
	TeamColor::ORANGE,
	TeamColor::BLUE,
	TeamColor::GREEN,
	TeamColor::PURPLE,
	TeamColor::PINK,
	TeamColor::YELLOW,
	TeamColor::CYAN
};

class Color {
public:
	static btVector4 GetPlayerColor(int index) {
		uint32_t col = (uint32_t)colors[index];

		float alpha = float(col & 0xFF) / 255.0;
		float blue = float((col >> 8) & 0xFF) / 255.0;
		float green = float((col >> 16) & 0xFF) / 255.0;
		float red = float((col >> 24) & 0xFF) / 255.0;

		// Create vec4 color
		return btVector4(red, green, blue, alpha);
	}

};