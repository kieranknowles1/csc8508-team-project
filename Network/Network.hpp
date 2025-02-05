#pragma once

#ifdef NETWORK_TEST
#include <iostream>

void DebugOut(const std::string& message) {
	std::cerr << "[DEBUG]: ";
	std::cerr << message << std::endl;
}

#endif
