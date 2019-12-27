#include "precompile.h"
#include "server.h"
#include "line.h"

server::server(window& win, point begin, int port, std::vector<ui_element*> elems) :

	tcp_server(port, &console_log),

	ui_element(win, begin),

	console_log(win,
		{ win.get_size().x / 2 + 1,3 },
		{ win.get_size().x / 2 - 2, win.get_size().y - 4 },
		true
	),

	console(win,
		{ 1,3 },
		{ win.get_size().x / 2 - 1, win.get_size().y - 4 },
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

void server::argument_handler(func_map fm, std::string args, std::string func)
{
	std::vector<std::string> arguments = argument_slicer(args);

	if (arguments.size() < 1) {
		console << "The syntax of the command is incorrect. try " << func << " -h\n";
		return;
	}

	for (int i = 0; i < arguments.size(); i += 2) {
		// in the vector: arg, value, arg1, value1
		// so we use loop with i += 2 to get every argument and value pair
		std::string argument = arguments[i];
		std::string value = "";
		if (arguments.size() >= 2) {
			std::string value = arguments[i + 1];
		}

		if (fm.count(argument)) { // evaluate argument
			fm[argument](value);
			return;
		}
		else if (argument == "-h") { // show help
			console << help[func] << "\n";
		}
		else { // invalid syntax
			console << "The syntax of the command is incorrect. try " << func << " -h\n";
		}
	}
}

void server::argument_parser(std::function<void(std::string)> f, std::string args, std::string func)
{
	if (args == "-h") {
		console << help[func] << "\n";
	}
	else {
		f(args);
	}
}

std::vector<std::string> server::argument_slicer(std::string args)
{
	std::istringstream iss(args);
	return std::vector<std::string>{std::istream_iterator<std::string>(iss), {}};
}
