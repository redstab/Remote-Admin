#pragma once
#include "precompile.h"
#include "server.h"
struct client_commands {
};
class server_commands {
public:
	void show(std::string args);
	void shutdown(std::string args);
	void scroll(std::string args);
	void help(std::string args);
	void attach(std::string args);
	void reconnect(std::string args);
	void disconnect(std::string args);

	friend class server;
};