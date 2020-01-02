#pragma once
#include "precompile.h"
#include "server.h"

func_map server::get_server_commands()
{
	return { 
		
		{ "show", [&](std::string args) {

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
		} },

		{ "reconnect", [&](std::string args) {
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
		} },

		{ "disconnect", [&](std::string args) {
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
		} }
	 };
}
