#include "precompile.h"
#include "tcp_client.h"

int main() // TODO: make this to class and fix a hash-table for identifier buffers for incoming pakets. add some cmd thingies. remote stuff and we are done. boom remote administration tool
{
	std::string ip_address = "127.0.0.1";			// IP Address of the server
	int port = 54321;						// Listening port # on the server

	std::locale::global(std::locale("sw"));

	std::string size;

	startup_wsa();

	tcp_client klient(ip_address, port);

	klient.set_disconnect_action([&]() {exit(-1); });

	klient.set_responses({

		{"time", [&](std::string data) {return std::to_string(time(0)); }},
		{"alive", [&](std::string data) {return "true"; }}

		});

	bool connected = false;

	while (!connected) {
		connected = klient.connect();
	}

	klient.paket_loop();

	//while (std::cin >> size) {
	//	int isize = std::stoi(size);
	//	std::string a;
	//	a.append(std::string(16 - size.length(), '0'));
	//	a.append(size);
	//	a.append(std::string(16 - size.length(), '0'));
	//	a.append(size);
	//	std::cout << a << std::endl;
	//	std::cin.get();
	//	send(sock, a.c_str(), 32, 0);
	//	char response[1];
	//	recv(sock, response, 1, 0);
	//	if (response[0] == '1') {
	//		send(sock, std::string(isize, 'a').c_str(), isize, 0);
	//		send(sock, std::string(isize, 'b').c_str(), isize, 0);
	//	}
	//}

}