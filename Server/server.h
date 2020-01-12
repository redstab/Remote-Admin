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

	std::unordered_map<std::string, std::string> global_help = {
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll <command|log> \n\n   scroll-up: arrow_up\n   scroll-down: arrow_down\n   scroll-done: enter\n"},
		{"hide", "hides the window/log from the screen\n\n  hide [log]\n\n   log : hides the log\n"},
		{"unhide", "unhides/shows the window/log\n\n  unhide [log]\n\n   log : shows the log\n"},
		{"help", "shows help documentation for every function\n"},
		{"clear", "clears and removes text from a window\n\n  clear [window]\n\n   windows:\n\n    log : clears the logger window [right]\n    this[default] : clears the command line [left]\n"},
		{"log", "sets the log level for the logger\n\n  log [level ...]\n\n   log-levels:\n\n    info : shows only the essential debug information\n    verbose : shows the more verbose debug information\n    vverbose : shows super verbose debug information\n    everything : shows messages from every log level\n"}
	};

	std::unordered_map<std::string, std::string> server_help = { // dictionary för att hantera hjälpen till server kommandon
		{"show","shows properties about the server.\n\n  show [property]\n\n   properties:\n    clients\n    time\n    info\n"},
		{"attach", "attaches to a client to interact with it \n\n  attach [name|ip|id]|[-p]\n   -p : pick client\n"},
		{"reconnect", "reconnect a specific client\n\n  reconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"},
		{"disconnect", "disconnect a specific client \n\n  disconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"}
	};

	std::unordered_map<std::string, std::string> klient_help = { // dictionary för att hantera hjälpen till klient kommandon
		{"detach", "detaches from a client to interact with the server instead\n"},
		{"reconnect", "reconnects the attached client\n"},
		{"disconnect", "disconnects the attached client\n"},
		{"process", "executes a process on klient\n\n  process [-h]|[-v] [path]|[-p]\n\n   -h : hidden\n   -v : visible\n   -p : gui pick executable\n   path : fullpath to executable\n"},
		{"download", "downloads a file from the attached client to the server\n\n  download [file] [folder] [-p]\n\n   file : path to file on client\n   folder : where to drop the file on the server   -p : pick path\n"},
		{"upload", "uploads a file from the server to the attached client\n\n  upload [file] [folder] [-p]\n\n   path : path to the file on server\n   folder : where to drop the file on the client   -p : pick path\n"},
		{"shell-process", "creates a interactive shell on the client\n\n  shell-process [shell] [wd]\n\n   shell : the shell application to run on the client\n   wd : the working directory of the executble\n"},
		{"get-info", "fetches information about the client. view by calling show-info\n\n"},
		{"show-info", "views the fetched information about the client\n\n  show-info [connection|computer|location|all]\n\n   connection : shows brief connectivity information about client\n   computer : shows information about the client computer\n   location : shows information about the current geolocation of the client\n   all : shows all the above information in a \"json\" like format\n"}
	};

	// det går inte att ha dessa mappar utanför instansen av klassen 
	// eftersom att de bygger på funktionalitet från tcp_server
	func_map get_server_commands();
	func_map get_client_commands();
	func_map get_global_commands();

	// för att hantera server specefika kommandon i konsolen
	func_map server_commands = get_server_commands();

	// för att hantera klient specefika kommandon i konsolen
	func_map client_commands = get_client_commands();

	func_map global_commands = get_global_commands();

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
	packet wait_response(std::string id_first, std::string id_second, client* owner); // väntar tills en respons från klienten finns i packet_queue

	std::pair<int,bool> pick_template(int max_elements, int top_offset, size max_size, std::function<void(int, int)> print_func, std::function<void()> disc_func);

	bool pick_file(std::filesystem::path root, std::string& buffer); // för att välja en fil hos klient
	bool pick_file_local(std::filesystem::path root, std::string& buffer); // för att välja en fil hos server
	bool pick_folder(std::filesystem::path root, std::string& buffer); // för att välja en mapp hos klient
	bool pick_folder_local(std::filesystem::path root, std::string& buffer); // för att välja en mapp hos server
	bool pick_client(client*& buffer); // för att välja en klient
};