#include "precompile.h"
#include "server.h"
#include "client-commands.h"
#include "server-commands.h"
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
	int cursor = 0; // index r�knare
	int offset = 0; // print offsett n�r det �r mer element i vektorn �n vad som kan visas p� sk�rmen, anv�nds vis skrollning 
	print_func(cursor, offset); // visa utg�ngspunkt 
	while (key != 13) { // medans vi inte har valt en fil

		key = getch();
		if (attached != nullptr && (attached->name.empty() || attached->ip_address.empty())) { // -> ESCAPE n�r klienten har f�rlorat anslutning
			key = 27;
		}
		if (key == KEY_DOWN) { // DOWN ARROW
			if ((cursor + 1) < max_elements) { // om cursor(index) �r mindre �n vektor l�ngden
				if ((cursor + 1 - offset) == max_size.y - 1) { // om den n�sta relativa cursor positionen �r under sk�rmen, �ka offset
					offset += 1;
				}
				print_func(++cursor, offset); // �ka cursor och printa igen
			}
		}
		else if (key == KEY_UP) { // UP ARROW
			if ((cursor - 1) >= 0) { // om n�sta cursor(index) �r st�rre �n 0 
				if ((cursor - 1 - offset) == -1 && offset > 0) { // om n�sta relativa cursor poision �r uvanf�r sk�rmen och att offset �r st�rre �n 0, minska offset
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

bool server::pick_file_attached(std::filesystem::path root, std::string& buffer)
{
	std::string file_buffer;

	if (send(*attached, { "select", "C:\\" })) {

		console.clear_element(); // "g�m" konsolen
		refresh();
		wrefresh(console.get_derived());
		wrefresh(window_.get_window());

		while (file_buffer.empty()) { // medans vi inte har n�gon fil i buffern
			std::vector<std::pair<std::filesystem::path, int>> items; // vektor buffer f�r mapp inneh�ll
			packet status = wait_response("dir_status", attached); // dir_status �r hur m�nga element som finns i mappen
			int amount = std::stoi(status.data); // konvertera till int
			delete_packet(status); // ta bort fr�n k�n
			for (int i = 0; i < amount; i++) // ta emot amount antal packet
			{
				packet file = wait_response("filedescription", attached); // m�ste ha id 'filedescription' och ha attached som �gare
				auto [path, size] = utility::ArgSplit(file.data, '|'); // c:\windows\system32\cmd.exe|231232 -> path[c:\windows\system32\cmd.exe], size[231232]
				items.push_back({ path, std::stoi(size) }); // l�gg till
				delete_packet(file); // ta bort fr�n k�n
			}

			int index = pick_template(items.size(), 1, console.get_element_size(),
				[&](int cursor, int offset) {utility::print_dir(items, attached->name, console.get_derived(), offset, cursor, console.get_element_size()); },
				[&] {
					if (!(attached->name.empty() || attached->ip_address.empty())) { // om klient �r ansluten
						send(*attached, { "dir_action", "done" }); // indikera f�r clienten att vi �r klara
					}
					else {
						client_commands["detach"]("");
					}
					console.draw_element(); // ta bort explorer ui som skrivs ovanp� derived_ f�nstret s� genom att skriva console s� �verskrivs derived med konsolen
				});

			if (index < 0 || attached->name.empty() || attached->ip_address.empty()) { // om klienten inte �r ansluten
				client_commands["detach"]("");
				console.draw_element(); // ta bort explorer ui som skrivs ovanp� derived_ f�nstret s� genom att skriva console s� �verskrivs derived med konsolen
				return false;
			}
			else {
				if (items[index].second == -9) { // om vi valde en undermapp
					send(*attached, { "goto", std::to_string(index) });// indikera att vi vill bes�ka en undermapp
				}
				else { // om vi valde en fil
					send(*attached, { "dir_action", "done" }); // indikera tt vi �r klara
					file_buffer = items[index].first.string(); // s�tt buffer till path
				}
			}

		}

		buffer = file_buffer;
		console.clear_element(); // "g�m" konsolen
		console.draw_element(); // ta bort explorer ui som skrivs ovanp� derived_ f�nstret s� genom att skriva console s� �verskrivs derived med konsolen
		return true; // returnera success

	}
	return false;
}

bool server::pick_client(client*& buffer)
{
	if (clients.get_list().size() != 0) {
		int index = pick_template(clients.get_list().size(), 0, console.get_element_size(),
			[&](int cursor, int offset) {utility::print_clients(clients.get_list(), console.get_derived(), offset, cursor, console.get_element_size()); },
			[&] {
				if (attached != nullptr && !(attached->name.empty() || attached->ip_address.empty())) { // om klient �r ansluten
					send(*attached, { "dir_action", "done" }); // indikera f�r clienten att vi �r klara
				}
				else {
					client_commands["detach"]("");
				}
				console.draw_element(); // ta bort explorer ui som skrivs ovanp� derived_ f�nstret s� genom att skriva console s� �verskrivs derived med konsolen
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

