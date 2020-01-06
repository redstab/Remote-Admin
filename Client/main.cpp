#include "precompile.h"
#include "client.h"

int main()
{
	std::string ip_address = "127.0.0.1";// IP Address of the server
	int port = 54321; // Listening port # on the server

	std::locale::global(std::locale("sw")); // s� att vi kan anv�nda ���

	startup_wsa(); // starta winsocket

	client main(ip_address, port); // initializera klient till ip och port

	main.connect(); // anslut

	main.run_packet_handler(); // k�r packet hanterar
}