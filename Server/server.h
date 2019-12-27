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

	void startup(); // för att starta paket tråden

	void cli_loop(); // loop för att ta emot kommandon
	
	// ärvda från ui_element
	size get_element_size() const; 
	void draw_element();

	~server();
private:

	std::unordered_map<std::string, std::string> help = { // dictionary för att hantera hjälpen till kommandon
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   options\n"},
		{"shutdown", "shuts down the server"}
	};

	func_map server_commands = { // för att hantera server specefika kommandon i konsolen

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
					console.dö();
				}
		, args, "shutdown"); }}

	};

	std::string default_prompt = "server $ ";

	window_log console_log; // för att hantera log (höger sida)
	command_line console; // för att hantera konsolen (vänster sida)
	wireframe wire; // för att hantera wireframen(linjer, titel)

	std::thread packet_thread; // tråd för att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argumnt till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar såsom -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces
};