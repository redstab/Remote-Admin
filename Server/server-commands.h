#pragma once
#include "precompile.h"
#include "server.h"

func_map server::get_server_commands()
{
	return {

		{ "show", [&](std::string args) {

			argument_handler({

				{"clients", [&](std::string value) {
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

				{"time", [&](std::string value) {
					console << str_time() << "\n";
				}},

				{"info", [&](std::string value) {
					console << " port: " << std::to_string(listen_port) << "\n listen_socket: " << std::to_string(listen_socket) << "\n number of clients: " << std::to_string(clients.get_list().size()) << "\n";
				}}

			}, args, "show");
		} },

		{ "shutdown", [&](std::string args) {
			argument_parser(
				[&](std::string value) {
					alive = false;
					console.dö();
			}, args, "shutdown");
		} },

		{ "scroll", [&](std::string args) {

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

		} },

		{ "help", [&](std::string args) {
			console << "\n";
			for (auto [func, doc] : server_help) {
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		} },

		{ "attach", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = nullptr;
				if (args == "-p") {
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else {
					klient = clients.search(args);
				}

				if (klient != nullptr) { // found
					attached = klient;
					console.set_prompt(attached->name + " $ ");
					console.set_functions(client_commands);
				}
				else { // !found
					console << "There is no such client connected to the server\n";
				}
			}, args, "attach");
		} },

		{ "reconnect", [&](std::string args) {
			argument_parser([&](std::string value) {
				client* klient = nullptr;
				if (args == "-p") {
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else {
					klient = clients.search(args);
				}
				if (klient != nullptr) { // found
					clients.disconnect_client(*klient);
					console << "Successfully disconnected client\n";
					console << "Client should connect soon..\n";
				}
				else if (args == "all") { // om man vill disconnecta alla klienter
					auto client_list = clients.get_list();
					for (auto c : client_list) {
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
				if (args == "-p") {
					if (pick_client(klient)) {
						console << "Picked ->" << klient->to_string() << "\n";
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else {
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

		{ "clear",[&](std::string args) {
			argument_parser([&](std::string value) {
				if (args == "log") {
					console_log.clear();
					console_log.draw_element();
				}
				else if (args == "this" || args == "") {
					console.clear();
					console.draw_element();
				}
				else {
					console << "Cannot clear a window that does not exsists\n";
				}
			}, args, "clear");
		}}
	};
}
