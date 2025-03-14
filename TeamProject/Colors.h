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
	PINK = 0xFF98BFFF,
	YELLOW = 0xFFFF00FF,
	CYAN = 0x00FFFFFF
};