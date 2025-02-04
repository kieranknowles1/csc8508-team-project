#include "Server.hpp"


Server::~Server() {
	if (m_thread != nullptr) delete m_thread;
}


void Server::Start(bool threaded) {
	if (threaded) {
#ifdef NETWORK_TEST
		DebugOut("Starting network on a separate thread.");
#endif

	}

#ifdef NETWORK_TEST
	DebugOut("Starting network on the main thread.");
#endif
}





void Server::Stop() {
#ifdef NETWORK_TEST
	DebugOut("Stopping server.");
#endif


#ifdef NETWORK_TEST
	DebugOut("Server stopped.");
#endif
}
