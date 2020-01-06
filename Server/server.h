#pragma once
#include "precompile.h"
#include "window_log.h"
#include "command_line.h"
#include "tcp_server.h"
#include "title.h"
#include "wireframe.h"

// för att hantera ui aspekten hos server applicationen 
// ärver från ui_element eftersom att den är en sorts element som visas på skärmen
// ärver från tcp_server eftersom att den här en typ av server 
class server :
	public ui_element,
	public tcp_server
{
public:
	server(window&, point, int, std::vector<ui_element*>); // konstruktor

	void startup(); // för att starta paket tråden

	void cli_loop(); // loop för att ta emot kommandon

	// virituella funktioner ärvda från ui_element
	size get_element_size() const;
	void draw_element();

	~server();
private:

	std::unordered_map<std::string, std::string> server_help = { // dictionary för att hantera hjälpen till server kommandon
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   time\n   info\n"},
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll <command|log> \n\n  scroll-up: arrow_up\n  scroll-down: arrow_down\n  scroll-done: enter\n"},
		{"attach", "attaches to a client to interact with it \n\n  attach [name|ip|id]|[-p]\n   -p : pick client\n"},
		{"reconnect", "reconnect a specific client\n\n  reconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"},
		{"disconnect", "disconnect a specific client \n\n  disconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"}
	};
	std::unordered_map<std::string, std::string> klient_help = { // dictionary för att hantera hjälpen till klient kommandon
		{"detach", "detaches from a client to interact with the server instead\n"},
		{"reconnect", "reconnects the attached client\n"},
		{"disconnect", "disconnects the attached client\n"},
		{"process", "executes a process on klient\n\n  process [-h]|[-v] [path]|[-p]\n\n   -h : hidden\n   -v : visible\n   -p : gui pick executable\n   path : fullpath to executable\n"},
		{"download", "downloads a file from the attached client\n\n  download [path]|[-p]\n\n   path : path to file\n   -p : pick path\n"}
	};

	// det går inte att ha dessa mappar utanför instansen av klassen 
	// eftersom att de bygger på funktionalitet från tcp_server
	func_map get_server_commands();
	func_map get_client_commands();

	// för att hantera server specefika kommandon i konsolen
	func_map server_commands = get_server_commands();

	// för att hantera klient specefika kommandon i konsolen
	func_map client_commands = get_client_commands();

	client* attached; // the currently attached client

	std::string default_prompt = "server $ ";

	window_log console_log; // för att hantera log (höger sida)
	command_line console; // för att hantera konsolen (vänster sida)
	wireframe wire; // för att hantera wireframen(linjer, titel)

	std::thread client_packet_thread; // tråd för att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argument till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar såsom -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces

	void scroll(window_log&); // använd för att skrolla fönstren

	packet wait_response(std::string id, client* owner); // väntar tills en respons från klienten finns i packet_queue

	int pick_template(int max_elements, int top_offset, size max_size, std::function<void(int, int)> print_func, std::function<void()> disc_func);

	bool pick_file_attached(std::filesystem::path root, std::string& buffer); // för att välja en fil
	bool pick_client(client*& buffer); // för att välja en fil
};