#include "precompile.h"
#include "server.h"
#include "client-commands.h"
#include "server-commands.h"
#include "global-commands.h"
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
		utility::add(server_commands, global_commands)
	),

	wire(elems)


{}

void server::startup()
{
	client_packet_thread = std::thread(&tcp_server::run, this);
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
	client_packet_thread.join();
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
			value = arguments[i + 1];
		}

		if (fm.count(argument)) { // evaluate argument
			fm[argument](value);
			return;
		}
		else if (argument == "-h") { // show help
			console << (attached == nullptr ? server_help[func] : klient_help[func]) << "\n";
		}
		else { // invalid syntax
			console << "The syntax of the command is incorrect. try " << func << " -h\n";
		}
	}
}

void server::argument_parser(std::function<void(std::string)> f, std::string args, std::string func)
{
	if (args == "-h") {
		console << (attached == nullptr ? server_help[func] : klient_help[func]) << "\n";
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

void server::scroll(window_log& win)
{
	int key = 0;
	while (key != 13) {
		key = getch();
		if (key == KEY_UP) {
			win.scroll(-1);
		}
		else if (key == KEY_DOWN) {
			win.scroll(1);
		}
		win.draw_element();
	}
}

packet server::wait_response(std::string id, client* owner)
{
	packet p;
	while (p.id != id && p.owner != owner) { // medans vi inte har hittat packetet
		for (auto c : packet_queue) {
			if (c.id == id && c.owner == owner) { // om paketet har [id] som id och [owner] som owner
				p = c; // paket hittat
			}
		}
	}
	return p; // returnera paket
}

int server::pick_template(int max_elements, int top_offset, size max_size, std::function<void(int, int)> print_func, std::function<void()> disc_func)
{
	int key = 0; // tangent
	int cursor = 0; // index räknare
	int offset = 0; // print offsett när det är mer element i vektorn än vad som kan visas på skärmen, används vis skrollning 
	print_func(cursor, offset); // visa utgångspunkt 
	while (key != 13) { // medans vi inte har tryckt på enter 

		key = getch();
		if (attached != nullptr && (attached->name.empty() || attached->ip_address.empty())) { // -> ESCAPE när klienten har förlorat anslutning
			key = 27;
		}
		if (key == KEY_DOWN) { // DOWN ARROW
			if ((cursor + 1) < max_elements) { // om cursor(index) är mindre än vektor längden
				if ((cursor + 1 - offset) == max_size.y - top_offset) { // om den nästa relativa cursor positionen är under skärmen, öka offset
					offset += 1;
				}
				print_func(++cursor, offset); // öka cursor och printa igen
			}
		}
		else if (key == KEY_UP) { // UP ARROW
			if ((cursor - 1) >= 0) { // om nästa cursor(index) är större än 0 
				if ((cursor - 1 - offset) == -1 && offset > 0) { // om nästa relativa cursor poision är uvanför skärmen och att offset är större än 0, minska offset
					offset -= 1;
				}
				print_func(--cursor, offset); // minska cursor och printa igen
			}
		}
		else if (key == 127) { // BACKSPACE
			cursor = 0;
			key = 13;
		}
		else if (key == 27) { // ESCAPE
			disc_func();
			return -1;
		}

	}
	return cursor;
}

bool server::pick_file(std::filesystem::path root, std::string& buffer)
{
	std::string file_buffer;

	if (send(*attached, {"select", root.string() })) {

		console.clear_element(); // "göm" konsolen
		refresh();
		wrefresh(console.get_derived());
		wrefresh(window_.get_window());

		while (file_buffer.empty()) { // medans vi inte har någon fil i buffern
			std::vector<std::pair<std::filesystem::path, int>> items; // vektor buffer för mapp innehåll
			packet status = wait_response("dir_status", attached); // dir_status är hur många element som finns i mappen
			int amount = std::stoi(status.data); // konvertera till int
			delete_packet(status); // ta bort från kön
			for (int i = 0; i < amount; i++) // ta emot amount antal packet
			{
				packet file = wait_response("filedescription", attached); // måste ha id 'filedescription' och ha attached som ägare
				auto [path, size] = utility::ArgSplit(file.data, '|'); // c:\windows\system32\cmd.exe|231232 -> path[c:\windows\system32\cmd.exe], size[231232]
				items.push_back({ path, std::stoi(size) }); // lägg till
				delete_packet(file); // ta bort från kön
			}

			int index = pick_template(items.size(), 1, console.get_element_size(),
				[&](int cursor, int offset) {utility::print_dir(items, attached->name, console.get_derived(), offset, cursor, console.get_element_size(), true); },
				[&] {
					if (!(attached->name.empty() || attached->ip_address.empty())) { // om klient är ansluten
						send(*attached, { "dir_action", "done" }); // indikera för clienten att vi är klara
					}
					else {
						client_commands["detach"]("");
					}
					console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				});

			if (index < 0 || attached->name.empty() || attached->ip_address.empty()) { // om klienten inte är ansluten
				client_commands["detach"]("");
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				return false;
			}
			else {
				if (items[index].second == -9) { // om vi valde en undermapp
					send(*attached, { "goto", std::to_string(index) });// indikera att vi vill besöka en undermapp
				}
				else { // om vi valde en fil
					send(*attached, { "dir_action", "done" }); // indikera tt vi är klara
					file_buffer = items[index].first.string(); // sätt buffer till path
				}
			}

		}

		buffer = file_buffer;
		console.clear_element(); // "göm" konsolen
		console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
		return true; // returnera success

	}
	return false;
}

bool server::pick_folder(std::filesystem::path root, std::string& buffer)
{
	std::string folder_buffer;

	if (send(*attached, { "select", root.string() })) {

		console.clear_element(); // "göm" konsolen
		refresh();
		wrefresh(console.get_derived());
		wrefresh(window_.get_window());

		while (folder_buffer.empty()) { // medans vi inte har någon fil i buffern
			std::vector<std::pair<std::filesystem::path, int>> items; // vektor buffer för mapp innehåll
			packet status = wait_response("dir_status", attached); // dir_status är hur många element som finns i mappen
			int amount = std::stoi(status.data); // konvertera till int
			delete_packet(status); // ta bort från kön
			for (int i = 0; i < amount; i++) // ta emot amount antal packet
			{
				packet file = wait_response("filedescription", attached); // måste ha id 'filedescription' och ha attached som ägare
				auto [path, size] = utility::ArgSplit(file.data, '|'); // c:\windows\system32\cmd.exe|231232 -> path[c:\windows\system32\cmd.exe], size[231232]
				items.push_back({ path, std::stoi(size) }); // lägg till
				delete_packet(file); // ta bort från kön
			}

			int index = pick_template(items.size(), 1, console.get_element_size(),
				[&](int cursor, int offset) {utility::print_dir(items, attached->name, console.get_derived(), offset, cursor, console.get_element_size(), false); },
				[&] {
					if (!(attached->name.empty() || attached->ip_address.empty())) { // om klient är ansluten
						send(*attached, { "dir_action", "done" }); // indikera för clienten att vi är klara
					}
					else {
						client_commands["detach"]("");
					}
					console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				});

			if (index < 0 || attached->name.empty() || attached->ip_address.empty()) { // om klienten inte är ansluten
				client_commands["detach"]("");
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				return false;
			}
			else {
				if (items[index].second == -9) { // om vi valde en undermapp
					send(*attached, { "goto", std::to_string(index) });// indikera att vi vill besöka en undermapp
				}
				else { // om vi valde en fil
					send(*attached, { "dir_action", "done" }); // indikera tt vi är klara
					folder_buffer = items[index].first.string(); // sätt buffer till path
				}
			}

		}

		buffer = folder_buffer;
		console.clear_element(); // "göm" konsolen
		console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
		return true; // returnera success

	}
	return false;
}

bool server::pick_client(client*& buffer)
{
	if (clients.get_list().size() != 0) {
		int index = pick_template(clients.get_list().size(), 2, console.get_element_size(),
			[&](int cursor, int offset) {utility::print_clients(clients.get_list(), console.get_derived(), offset, cursor, console.get_element_size()); },
			[&] {
				if (attached != nullptr && !(attached->name.empty() || attached->ip_address.empty())) { // om klient är ansluten
					send(*attached, { "dir_action", "done" }); // indikera för clienten att vi är klara
				}
				else {
					client_commands["detach"]("");
				}
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
			});
		if (index >= 0 && index < clients.get_list().size()) {
			buffer = &clients.get_list()[index];
			return true;
		}
		else {
			return false;
		}
	}
	else {
		console << "No clients\n";
		return false;
	}
}

