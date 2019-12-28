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
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   time\n   info\n"},
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll-up: arrow_up\n  scroll-down: arrow_down\n  scroll-done: enter\n"}
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
				}},

				{"info", [&](std::string value) {
					console << " port: " << std::to_string(listen_port) << "\n listen_socket: " << std::to_string(listen_socket) << "\n number of clients: " << std::to_string(clients.get_list().size()) << "\n";
				}}

			}, args, "show");
		}},

		{"shutdown", [&](std::string args) {
			argument_parser(
				[&](std::string value) {
					alive = false;
					console.dö();
				}
		, args, "shutdown"); }},

		{"scroll", [&](std::string args) {

			argument_handler({

				{"log", [&](std::string value) {
					console << "scrolling " << args;
					scroll(console_log);
					console << " -> done\n";
				}},

				{"command", [&](std::string value) {
					console << "scrolling " << args;
					scroll(console);
					console << " -> done\n";
				}}

			}, args, "scroll");

		}},

		{"help", [&](std::string args) {
			console << "\n";
			for (auto [func, doc] : help) {
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"attach", [&](std::string args) {
			client* klient = clients.search(args);
			if (klient != nullptr) {
				attached = klient;
				console.set_prompt(attached->name + " $ ");
				console.set_functions(client_commands);
			}
			else {
				console << "There is no such client connected to the server\n";
			}
		}},

	};

	func_map client_commands = {
		{"detach", [&](std::string args) {
			attached = nullptr;
			console.set_prompt(default_prompt);
			console.set_functions(server_commands);
		}}
	};

	client* attached; // the currently attached client

	std::string default_prompt = "server $ ";

	window_log console_log; // för att hantera log (höger sida)
	command_line console; // för att hantera konsolen (vänster sida)
	wireframe wire; // för att hantera wireframen(linjer, titel)

	std::thread packet_thread; // tråd för att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argumnt till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar såsom -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces

	void scroll(window_log&); // använd för att skrolla fönstren
};