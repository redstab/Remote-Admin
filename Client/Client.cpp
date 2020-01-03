#include "precompile.h"
#include "client.h"
#include "commands.h"

client::client(std::string ip, int port) : 
	ip_{ ip }, 
	port_{port}, 
	client_implentation(ip_, port_)
{
	client_implentation.set_disconnect_action([&] {startup(); });
	client_implentation.set_actions(actions_);
	client_implentation.set_responses(responses_);
}

void client::connect()
{
	bool connected = false;
	int tries = 0;

	std::cout << "connect()[@" << tries << "]";

	while (!connected) {
		connected = client_implentation.connect();
		tries += 1;
		std::cout << std::string(std::to_string(tries-1).length()+1, '\b') <<  tries << "]";
	}

	std::cout << " - " << Error(0) << std::endl;

	// Nu måste klienten vara ansluten till servern

}

void client::startup()
{
	std::cout << "disconnect() - " << Error(0) << std::endl;
	client_implentation = tcp_client(ip_, port_);
	client_implentation.set_disconnect_action([&] {startup(); });
	client_implentation.set_actions(actions_);
	client_implentation.set_responses(responses_);
	std::cout << "attempting_reconnect()\n";
	connect();
}
