#include <stdexcept>
#include <iostream>

#include "Server.hpp"

int main(int argc, char** argv) {
	Server server;
	server.Start();
	server.Stop();
}
