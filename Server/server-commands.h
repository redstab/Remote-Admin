#pragma once
#include "precompile.h"
#include "server.h"

func_map server::get_server_commands()
{
	return {

		{ "show", [&](std::string args) { // för att visa aspekter hos servern

			argument_handler({

				{"clients", [&](std::string value) { // visa klienter
					if (!clients.get_list().empty()) {
						console << "{\n";
						for (auto c : clients.get_list()) {
							console << "  " << c.to_string() << "\n";
						}
						console << "}\n";
					}
					else {
						console << "There are no clients connected to the server!\n";
					}
				}},

				{"time", [&](std::string value) { // visa tiden för att tolka window_log loggar
					console << str_time() << "\n";
				}},

				{"info", [&](std::string value) { // visa inställningar
					console << " port: " << std::to_string(listen_port) << "\n listen_socket: " << std::to_string(listen_socket) << "\n number of clients: " << std::to_string(clients.get_list().size()) << "\n";
				}}

			}, args, "show");
		} },

		{ "shutdown", [&](std::string args) { // stäng ner servern 
			argument_parser(
				[&](std::string value) {
					alive = false; // döda packet och client hanteraren
					console.dö(); // döda konsolen (bara en fancy animation)
			}, args, "shutdown");
		} },

		{ "scroll", [&](std::string args) { // skrolla ett fönster

			argument_handler({

				{"log", [&](std::string value) { // skrolla log
					console << "scrolling " << args;
					scroll(console_log);
					console << " -> done\n";
				}},

				{"command", [&](std::string value) { // skrolla konsolen
					console << "scrolling " << args;
					scroll(console);
					console << " -> done\n";
				}}

			}, args, "scroll");

		} },

		{ "help", [&](std::string args) { // visa hjälp
			console << "\n";
			for (auto [func, doc] : server_help) {
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		} },

		{ "attach", [&](std::string args) { // attacha en klient till attached
			argument_parser([&](std::string value) {
				client* klient = nullptr;
				if (args == "-p") { // om man vill välja en klient
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else { // annars sök efter argumentet som en klient
					klient = clients.search(args);
				}

				if (klient != nullptr) { // found
					attached = klient;
					console.set_prompt(attached->name + " $ "); // uppdatera prompt
					console.set_functions(client_commands); // uppdatera funktioner
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "attach");
		} },

		{ "reconnect", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = nullptr;
				if (args == "-p") { // om man vill välja en klient
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else { // annars sök efter argumentet som en klient
					klient = clients.search(args);
				}
				if (klient != nullptr) { // found
					clients.disconnect_client(*klient);
					console << "Successfully disconnected client\n";
					console << "Client should connect soon..\n";
				}
				else if (args == "all") { // om man vill omanluta alla klienter
					auto client_list = clients.get_list();
					for (auto c : client_list) {
						console_log << str_time() << " disconnect() - [" << std::to_string(c.socket_id) << "|" << c.ip_address << "]\n";
						clients.disconnect_client(c);
					}
					console << "Successfully disconnected all clients\n";
					console << "Clients should connect soon..\n";
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "disconnect");
		} },

		{ "disconnect", [&](std::string args) { 
			argument_parser([&](std::string value) {
				client* klient = nullptr;
				if (args == "-p") { // om man vill välja en klient
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else {// annars sök efter argumentet som en klient
					klient = clients.search(args);
				}

				if (klient != nullptr) { // found
					send(*klient, { "exit", " " });
					clients.disconnect_client(*klient);
					console << "Successfully disconnected client\n";
				}
				else if (args == "all") { // om man vill disconnecta alla klienter
					while(!clients.get_list().empty()) { // loopa tills client_listan är tom eller när alla klienter har förlorat anslutningen 
						client c = clients.get_list().front();
						send(c, { "exit", " " });
						console_log << str_time() << " disconnect() - [" << std::to_string(c.socket_id) << "|" << c.ip_address << "]\n";
						clients.disconnect_client(c);
					}
					console << "Successfully disconnected all clients\n";
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "disconnect");
		} },

		{ "clear",[&](std::string args) { // rensa ett fönster
			argument_parser([&](std::string value) {
				if (args == "log") { // rensa loggen 
					console_log.clear();
					console_log.draw_element();
				}
				else if (args == "this" || args == "") { // rensa konsolen
					console.clear();
					console.draw_element();
				}
				else { // args inte en konsol eller fönster
					console << "Cannot clear a window that does not exsists\n";
				}
			}, args, "clear");
		}}
	};
}
