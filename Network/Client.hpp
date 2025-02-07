#pragma once

#include "Network.hpp"


class Client : public Network {
public:
	Client();
	~Client();

private:
	ENetPeer* m_server;
};
