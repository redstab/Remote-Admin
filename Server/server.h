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

	void startup();

	void cli_loop();

	size get_element_size() const;
	void draw_element();

	~server();
private:

	func_map server_commands = {

		{"show", [this](std::string args) {
		if (args == "clients") {
			console << "{\n";
			for (auto c : clients.get_list()) {
				console << "  " << c.to_string() << "\n";
			}
			console << "}\n";
		}
		else {
			console << "The syntax of the command is incorrect. try show -h\n";
		}
		}},

		{"shutdown", [this](std::string args) {
			alive = false;
			console.dö();
		}}

	};

	window_log console_log;
	command_line console;
	wireframe wire;

	std::thread packet_thread;
};