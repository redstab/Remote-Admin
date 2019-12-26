#pragma once
#include "precompile.h"
#include "window_log.h"
#include "command_line.h"
#include "tcp_server.h"

class server :
	public ui_element
{
public:
	//server(std::string, int, int, )


private:
	const int padding;



	window_log console_log;
	command_line console;
	tcp_server tcp;

	func_map server_commands = {

	};
};