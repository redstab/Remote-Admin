#include "precompile.h"
#include "server.h"
#include "client-commands.h"
#include "server-commands.h"
#include "global-commands.h"
#include "line.h"
#include "directory.h"

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

	for (int i = 0; i < static_cast<int>(arguments.size()); i += 2) {
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
			console << utility::add(global_help, (attached == nullptr ? server_help : klient_help))[func] << "\n";
		}
		else { // invalid syntax
			console << "The syntax of the command is incorrect. try " << func << " -h\n";
		}
	}
}

void server::argument_parser(std::function<void(std::string)> f, std::string args, std::string func)
{
	if (args == "-h") {
		console << utility::add(global_help, (attached == nullptr ? server_help : klient_help))[func] << "\n";
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
	while (p.id != id && p.owner != owner && !owner->ip_address.empty()) { // medans vi inte har hittat packetet
		for (auto c : packet_queue) {
			if (c.id == id && c.owner == owner) { // om paketet har [id] som id och [owner] som owner
				p = c; // paket hittat
			}
		}
	}
	return p; // returnera paket
}

packet server::wait_response(std::string id_first, std::string id_second, client* owner)
{
	packet p;
	while ((p.id != id_first || p.id != id_second) && p.owner != owner && !owner->ip_address.empty()) { // medans vi inte har hittat packetet
		for (auto c : packet_queue) {
			if ((c.id == id_first || c.id == id_second) && c.owner == owner) { // om paketet har [id] som id och [owner] som owner
				p = c; // paket hittat
			}
		}
	}
	return p; // returnera paket
}

std::pair<int, bool> server::pick_template(int max_elements, int top_offset, size max_size, std::function<void(int, int)> print_func, std::function<void()> disc_func)
{
	int key = 0; // tangent
	int cursor = 0; // index räknare
	int offset = 0; // print offsett när det är mer element i vektorn än vad som kan visas på skärmen, används vis skrollning 
	print_func(cursor, offset); // visa utgångspunkt 
	while (key != 13 && key != KEY_RIGHT) { // medans vi inte har tryckt på enter 

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
			return { -1, false };
		}

	}
	return { cursor, (key != 13 ? true : false) };
}

bool server::pick_file(std::filesystem::path root, std::string& buffer)
{
	std::string path = root.string();

	console_log.Log<LOG_INFO>(logger() << str_time() << " pick() - init\n");

	while (buffer.empty() && !attached->ip_address.empty()) { // medans vi inte har valt något och inte har förlorat anslutning till klient

		if (send(*attached, { "folder-index", path })) { // fråga om indexering av mapp

			console_log.Log<LOG_VERBOSE>(logger() << str_time() << " index() - " << Error(0).to_string() << "\n");

			std::vector<std::pair<std::filesystem::path, int>> folder_items;

			packet folder = wait_response("response|folder-index", attached); // vänta på svar

			std::string seperated_list = folder.data; // representerar | separerad lista med filer och mappar

			delete_packet(folder); // ta bort svar

			// ta första elementet i listan och lägg till i mapp vektorn och gör detta tills att det inte finns något kvar
			// a|b|c|d -> b|c|d -> c|d -> d -> _
			while (!seperated_list.empty()) { // medans | separerade listan inte är tom

				auto [folder_item, rest] = utility::ArgSplit(seperated_list, '|'); // splita element

				auto [item, size] = utility::ArgSplit(folder_item, '*'); // splita element till path och storlek

				folder_items.push_back({ item, std::stoi(size) }); // lägg till i vektor

				seperated_list = rest; // nya listan som ska separeras blir den reseterande delen av nuvarande listan
			}

			auto pick_result = pick_template(folder_items.size(), 1, console.get_element_size(),
				[&](int cursor, int offset) { utility::print_dir(folder_items, "File to download from " + attached->name, console.get_derived(), offset, cursor, console.get_element_size()); },
				[&]
				{
					console << "(Status) Failed: Client Disconnected\n";
					console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				});
			
			console_log.Log<LOG_VERBOSE>(logger() << str_time() << " pick()[" << std::to_string(pick_result.first) << "] - " << Error(0).to_string() << "\n");

			if (pick_result.first == -1 || attached->ip_address.empty()) { // klient dc
				return false;
			}
			else if(folder_items[pick_result.first].second == -9) { // om det valda elementet är en mapp[size = -9] så går vi in i den
				path = std::filesystem::path(path + "\\" + folder_items[pick_result.first].first.filename().string()).lexically_normal().string(); // uppdatera path med den nya pathen
				console_log.Log<LOG_VERBOSE>(logger() << str_time() << " descend()[" << path << "] - ok\n");
			}
			else { // annars måste det valda elementet vara en fil då väljer vi den
				buffer = folder_items[pick_result.first].first.string();
				console_log.Log<LOG_VERBOSE>(logger() << str_time() << " pick()[" << buffer << "] - ok\n");
				return true;
			}
		}

	}
	return false;
}

bool server::pick_file_local(std::filesystem::path root, std::string& buffer)
{
	std::string file_buffer;
	console.clear_element(); // "göm" konsolen
	refresh();
	wrefresh(console.get_derived());
	wrefresh(window_.get_window());

	directory explorer(root.string());

	while (file_buffer.empty()) { // medans vi inte har någon fil i buffern
		int index = pick_template(explorer.current_directory().size(), 1, console.get_element_size(),
			[&](int cursor, int offset) { utility::print_dir(explorer.current_directory(), "File to upload to " + attached->name, console.get_derived(), offset, cursor, console.get_element_size()); },
			[&]
			{
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
			}).first;
		if (index < 0) {
			return false;
		}
		if (explorer.current_directory()[index].second == -9) { // om vi valde en undermapp
			explorer.descend(index);
		}
		else {
			file_buffer = explorer.current_directory()[index].first.string(); // sätt buffer till path
		}
	}

	buffer = file_buffer;
	console.clear_element(); // "göm" konsolen
	console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
	return true; // returnera success
}

bool server::pick_folder(std::filesystem::path root, std::string& buffer)
{
	std::string path = root.string();
	console_log.Log<LOG_INFO>(logger() << str_time() << " pick() - init\n");

	while (buffer.empty() && !attached->ip_address.empty()) { // medans vi inte har valt något och inte har förlorat anslutning till klient

		if (send(*attached, { "folder-index", path })) { // fråga om indexering av mapp
			
			console_log.Log<LOG_VERBOSE>(logger() << str_time() << " index() - " << Error(0).to_string() << "\n");

			std::vector<std::pair<std::filesystem::path, int>> folder_items;

			packet folder = wait_response("response|folder-index", attached); // vänta på svar

			std::string seperated_list = folder.data; // representerar | separerad lista med filer och mappar

			delete_packet(folder); // ta bort svar

			// ta första elementet i listan och lägg till i mapp vektorn och gör detta tills att det inte finns något kvar
			// a|b|c|d -> b|c|d -> c|d -> d -> _
			while (!seperated_list.empty() && !attached->ip_address.empty()) { // medans | separerade listan inte är tom

				auto [folder_item, rest] = utility::ArgSplit(seperated_list, '|'); // splita element

				auto [item, size] = utility::ArgSplit(folder_item, '*'); // splita element till path och storlek

				folder_items.push_back({ item, std::stoi(size) }); // lägg till i vektor

				seperated_list = rest; // nya listan som ska separeras blir den reseterande delen av nuvarande listan
			}

			auto pick_result = pick_template(folder_items.size(), 1, console.get_element_size(),
				[&](int cursor, int offset) { utility::print_dir(folder_items, "Folder to drop file to in " + attached->name, console.get_derived(), offset, cursor, console.get_element_size()); },
				[&]
				{
					console << "(Status) Failed: Client Disconnected\n";
					console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
				});

			console_log.Log<LOG_VERBOSE>(logger() << str_time() << " pick()[" << std::to_string(pick_result.first) << "] - " << Error(0).to_string() << "\n");
			
			if (pick_result.first == -1 || attached->ip_address.empty()) { // klient dc
				console << "(Status) Failed: Client Disconnected\n";
				return false;
			}
			else if (folder_items[pick_result.first].second == -9) { // om det valda elementet är en mapp[size = -9] så går vi in i den
				if (pick_result.second) {
					path = std::filesystem::path(path + "\\" + folder_items[pick_result.first].first.filename().string()).lexically_normal().string(); // uppdatera path med den nya pathen
					console_log.Log<LOG_VERBOSE>(logger() << str_time() << " descend()[" << path << "] - ok\n");
				}
				else {
					buffer = folder_items[pick_result.first].first.lexically_normal().string(); // returnera med valda pathen
					console_log.Log<LOG_VERBOSE>(logger() << str_time() << " pick()[" << path << "] - ok\n");
					return true;
				}
			}
		}

	}
	return false;
}

bool server::pick_folder_local(std::filesystem::path root, std::string& buffer)
{
	std::string folder_buffer;
	console.clear_element(); // "göm" konsolen
	refresh();
	wrefresh(console.get_derived());
	wrefresh(window_.get_window());

	directory explorer(root.string());

	while (folder_buffer.empty()) { // medans vi inte har någon fil i buffern
		auto index = pick_template(explorer.current_directory().size(), 1, console.get_element_size(),
			[&](int cursor, int offset) { utility::print_dir(explorer.current_directory(), "Download file from " + attached->name + " to folder", console.get_derived(), offset, cursor, console.get_element_size()); },
			[&]
			{
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
			});
		if (index.first < 0) {
			return false;
		}
		if (explorer.current_directory()[index.first].second == -9) { // om vi valde en undermapp
			if (index.second) {
				explorer.descend(index.first);
			}
			else {
				folder_buffer = explorer.current_directory()[index.first].first.string(); // sätt buffer till path
			}
		}
	}
	buffer = folder_buffer;
	console.clear_element(); // "göm" konsolen
	console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
	return true; // returnera success
}

bool server::pick_client(client*& buffer)
{
	if (clients.get_list().size() != 0) {
		int index = pick_template(clients.get_list().size(), 2, console.get_element_size(),
			[&](int cursor, int offset) { utility::print_clients(clients.get_list(), console.get_derived(), offset, cursor, console.get_element_size()); },
			[&]
			{
				if (attached != nullptr && !(attached->name.empty() || attached->ip_address.empty())) { // om klient är ansluten
					send(*attached, { "dir_action", "done" }); // indikera för clienten att vi är klara
				}
				else {
					client_commands["detach"]("");
				}
				console.draw_element(); // ta bort explorer ui som skrivs ovanpå derived_ fönstret så genom att skriva console så överskrivs derived med konsolen
			}).first;
		if (index >= 0 && index < static_cast<int>(clients.get_list().size())) {
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

