#include "precompile.h"
#include "server.h"
#include "line.h"

server::server(window& win, point begin, int port, std::vector<ui_element*> elems) :

	tcp_server(port, &console_log),

	ui_element(win,begin),

	console_log(win,
		{ win.get_size().x / 2 + 1,3 },
		{ win.get_size().x / 2 - 2, win.get_size().y - 4 },
		true
	),

	console(win,
		{ 1,3 },
		{ win.get_size().x/2 - 1, win.get_size().y - 4 },
		"server $ ",
		server_commands
	),

	wire(elems)

{}

void server::startup()
{
	packet_thread = std::thread(&tcp_server::run, this);
}

void server::cli_loop()
{
	while (console.alive()) {
		console.prompt();
	}
}

size server::get_element_size() const
{
	return window_.get_size();
}

void server::draw_element()
{
	wire.draw();
	console_log.draw_element();
	console.draw_element();
}

server::~server()
{
	packet_thread.join();
}
