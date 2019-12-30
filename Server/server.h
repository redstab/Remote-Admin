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
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   time\n   info\n"},
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll-up: arrow_up\n  scroll-down: arrow_down\n  scroll-done: enter\n"}
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
					console.d�();
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
		}},

		{"send", [&](std::string args) {

			message msg;
			auto found = args.find_first_of(' ');
			if (found != std::string::npos) {
				msg.identifier = args.substr(0, found);
				msg.data = args.substr(found+1);
			}
			else {
				msg.identifier = args;
				msg.data = ":>";
			}

			if (send(*attached, msg)) {
				console  << msg.identifier << "(" << msg.data << ") ->" << attached->name << "\n";

				packet paket;
				bool found = false;
				while (!found) {
					for (auto& p : packet_queue) {
						if (p.id == "response|" + msg.identifier && p.owner == attached) {
							paket = p;
							found = true;
						}
					}
				}
				// found proper response

				console << attached->name << " -> [(" << paket.id << ")|(" << paket.data << ")]\n";
				
				//remove paket
				packet_queue.erase(std::remove(packet_queue.begin(), packet_queue.end(), paket), packet_queue.end());

			}
			else {
				console << "Could not send request " << args << "\n";
			}


		}}
	};

	client* attached; // the currently attached client

	std::string default_prompt = "server $ ";

	window_log console_log; // f�r att hantera log (h�ger sida)
	command_line console; // f�r att hantera konsolen (v�nster sida)
	wireframe wire; // f�r att hantera wireframen(linjer, titel)

	std::thread packet_thread; // tr�d f�r att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argumnt till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar s�som -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces

	void scroll(window_log&); // anv�nd f�r att skrolla f�nstren
};