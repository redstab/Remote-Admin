#include "precompile.h"
#include "client.h"
#include "commands.h"

client::client(std::string ip, int port) : //simpel konstruktor
	ip_{ ip }, 
	port_{port}, 
	client_implementation(ip_, port_)
{
	client_implementation.set_disconnect_action([&] {startup(); });
	client_implementation.set_actions(actions_);
	client_implementation.set_responses(responses_);
}

void client::connect()
{
	bool connected = false;
	int tries = 0;

	std::cout << "connect()[@" << tries << "]";

	while (!connected) { // medans inte ansluten
		connected = client_implementation.connect(); //försök ansluta 
		tries += 1; // lägg till ett försök
		std::cout << std::string(std::to_string(tries-1).length()+1, '\b') <<  tries << "]"; // skriv antal försök
	}

	std::cout << " - " << Error(0) << std::endl;

	// Nu måste klienten vara ansluten till servern

}

void client::startup() //kallas vid förlorad anslutning
{
	std::cout << "disconnect() - " << Error(0) << std::endl;
	client_implementation = tcp_client(ip_, port_); //skapa ny instans
	client_implementation.set_disconnect_action([&] {startup(); });
	client_implementation.set_actions(actions_);
	client_implementation.set_responses(responses_);
	std::cout << "attempting_reconnect()\n";
	connect(); // anslut
}
