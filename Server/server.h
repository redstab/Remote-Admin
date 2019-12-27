#pragma once
#include "precompile.h"
#include "window_log.h"
#include "command_line.h"
#include "tcp_server.h"
#include "title.h"
#include "wireframe.h"

class server :
	public ui_element,
	public tcp_server
{
public:
	server(window&, point, int, std::vector<ui_element*>);

	void startup(); // f�r att starta paket tr�den

	void cli_loop(); // loop f�r att ta emot kommandon
	
	// �rvda fr�n ui_element
	size get_element_size() const; 
	void draw_element();

	~server();
private:

	std::unordered_map<std::string, std::string> help = { // dictionary f�r att hantera hj�lpen till kommandon
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   options\n"},
		{"shutdown", "shuts down the server"}
	};

	func_map server_commands = { // f�r att hantera server specefika kommandon i konsolen

		{"show", [&](std::string args) {
			argument_handler({

				{"clients", [&](std::string value) {
					console << "{\n";
					for (auto c : clients.get_list()) {
						console << "  " << c.to_string() << "\n";
					}
					console << "}\n";
				}},

				{"time", [&](std::string value) {
					console << str_time() << "\n";
				}}

		}, args, "show"); }},

		{"shutdown", [this](std::string args) {
			argument_parser(
				[&](std::string value) {
					alive = false;
					console.d�();
				}
		, args, "shutdown"); }}

	};

	std::string default_prompt = "server $ ";

	window_log console_log; // f�r att hantera log (h�ger sida)
	command_line console; // f�r att hantera konsolen (v�nster sida)
	wireframe wire; // f�r att hantera wireframen(linjer, titel)

	std::thread packet_thread; // tr�d f�r att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argumnt till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar s�som -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces
};