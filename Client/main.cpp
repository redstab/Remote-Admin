#include "precompile.h"
#include "client.h"

int main() // TODO: make this to class and fix a hash-table for identifier buffers for incoming pakets. add some cmd thingies. remote stuff and we are done. boom remote administration tool
{
	std::string ip_address = "127.0.0.1";// IP Address of the server
	int port = 54321; // Listening port # on the server

	std::locale::global(std::locale("sw"));

	startup_wsa();

	client main(ip_address, port);

	main.connect();

	main.run();
}