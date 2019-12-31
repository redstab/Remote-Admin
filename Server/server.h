#pragma once
#include "precompile.h"
#include "window_log.h"
#include "command_line.h"
#include "tcp_server.h"
#include "title.h"
#include "function_map.h"
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

	std::unordered_map<std::string, std::string> server_help = { // dictionary för att hantera hjälpen till server kommandon
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   time\n   info\n"},
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll <command|log> \n\n  scroll-up: arrow_up\n  scroll-down: arrow_down\n  scroll-done: enter\n"},
		{"attach", "attaches to a client to interact with it \n\n  attach [name|ip|id]\n"},
		{"reconnect", "reconnect a specific client\n\n  reconnect [name|id|ip|<all>]\n"},
		{"disconnect", "disconnect a specific client \n\n  disconnect [name|id|ip|<all>]\n"}
	};
	std::unordered_map<std::string, std::string> klient_help = { // dictionary för att hantera hjälpen till klient kommandon
		{"detach", "detaches from a client to interact with the server instead"},
		{"reconnect", "reconnects the attached client"},
		{"disconnect", "disconnects the attached client"},

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
			}, args, "shutdown");
		}},

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
			for (auto [func, doc] : server_help) {
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"attach", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = clients.search(args);
				if (klient != nullptr) { // found
					attached = klient;
					console.set_prompt(attached->name + " $ ");
					console.set_functions(client_commands);
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "attach");
		}},

		{"reconnect", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = clients.search(args);
				if (klient != nullptr) { // found
					clients.disconnect_client(*klient);
					console << "Successfully disconnected client\n";
					console << "Client should connect soon..\n";
				}
				else if (args == "all") { // om man vill disconnecta alla klienter
					for (auto c : clients.get_list()) {
						clients.disconnect_client(c);
					}
					console << "Successfully disconnected all clients\n";
					console << "Clients should connect soon..\n";
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "disconnect");
		}},
		
		{"disconnect", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = clients.search(args);
				if (klient != nullptr) { // found
					send(*klient, { "exit", " " });
					clients.disconnect_client(*klient);
					console << "Successfully disconnected client\n";
				}
				else if (args == "all") { // om man vill disconnecta alla klienter
					for (auto c : clients.get_list()) {
						send(c, { "exit", " " });
						clients.disconnect_client(c);
					}
					console << "Successfully disconnected all clients\n";
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "disconnect");
		}}

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
				msg.data = args.substr(found + 1);
			}
			else {
				msg.identifier = args;
				msg.data = ":>";
			}

			if (send(*attached, msg)) {
				console << msg.identifier << "(" << msg.data << ") ->" << attached->name << "\n";

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

	window_log console_log; // för att hantera log (höger sida)
	command_line console; // för att hantera konsolen (vänster sida)
	wireframe wire; // för att hantera wireframen(linjer, titel)

	std::thread packet_thread; // tråd för att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argument till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar såsom -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces

	void scroll(window_log&); // använd för att skrolla fönstren

	friend class function_map<func_map, server>;
};