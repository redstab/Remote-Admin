#pragma once
#include "precompile.h"
#include "window_log.h"
#include "command_line.h"
#include "tcp_server.h"
#include "title.h"
#include "wireframe.h"

// f�r att hantera ui aspekten hos server applicationen 
// �rver fr�n ui_element eftersom att den �r en sorts element som visas p� sk�rmen
// �rver fr�n tcp_server eftersom att den h�r en typ av server 
class server :
	public ui_element,
	public tcp_server
{
public:
	server(window&, point, int, std::vector<ui_element*>); // konstruktor

	void startup(); // f�r att starta paket tr�den

	void cli_loop(); // loop f�r att ta emot kommandon

	// virituella funktioner �rvda fr�n ui_element
	size get_element_size() const;
	void draw_element();

	~server();
private:

	std::unordered_map<std::string, std::string> server_help = { // dictionary f�r att hantera hj�lpen till server kommandon
		{"show","shows properties about the server.\n\n show [property][-h]\n\n  properties:\n   clients\n   time\n   info\n"},
		{"shutdown", "shuts down the server or [aborts]\n"},
		{"scroll", "scrolls a window up or down\n\n  scroll <command|log> \n\n  scroll-up: arrow_up\n  scroll-down: arrow_down\n  scroll-done: enter\n"},
		{"attach", "attaches to a client to interact with it \n\n  attach [name|ip|id]|[-p]\n   -p : pick client\n"},
		{"reconnect", "reconnect a specific client\n\n  reconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"},
		{"disconnect", "disconnect a specific client \n\n  disconnect [name|id|ip|<all>]|-p\n   -p : pick client\n"}
	};
	std::unordered_map<std::string, std::string> klient_help = { // dictionary f�r att hantera hj�lpen till klient kommandon
		{"detach", "detaches from a client to interact with the server instead\n"},
		{"reconnect", "reconnects the attached client\n"},
		{"disconnect", "disconnects the attached client\n"},
		{"process", "executes a process on klient\n\n  process [-h]|[-v] [path]|[-p]\n\n   -h : hidden\n   -v : visible\n   -p : gui pick executable\n   path : fullpath to executable\n"},
		{"download", "downloads a file from the attached client\n\n  download [path]|[-p]\n\n   path : path to file\n   -p : pick path\n"}
	};

	// det g�r inte att ha dessa mappar utanf�r instansen av klassen 
	// eftersom att de bygger p� funktionalitet fr�n tcp_server
	func_map get_server_commands();
	func_map get_client_commands();

	// f�r att hantera server specefika kommandon i konsolen
	func_map server_commands = get_server_commands();

	// f�r att hantera klient specefika kommandon i konsolen
	func_map client_commands = get_client_commands();

	client* attached; // the currently attached client

	std::string default_prompt = "server $ ";

	window_log console_log; // f�r att hantera log (h�ger sida)
	command_line console; // f�r att hantera konsolen (v�nster sida)
	wireframe wire; // f�r att hantera wireframen(linjer, titel)

	std::thread client_packet_thread; // tr�d f�r att hantera paket och nya klienter

	void argument_handler(func_map fm, std::string args, std::string); // hantera lokala argument till funktioner
	void argument_parser(std::function<void(std::string)> f, std::string, std::string); // hantera globala parametrar s�som -h och -t
	std::vector<std::string> argument_slicer(std::string args); // splice arguments by spaces

	void scroll(window_log&); // anv�nd f�r att skrolla f�nstren

	packet wait_response(std::string id, client* owner); // v�ntar tills en respons fr�n klienten finns i packet_queue

	int pick_template(int max_elements, int top_offset, size max_size, std::function<void(int, int)> print_func, std::function<void()> disc_func);

	bool pick_file_attached(std::filesystem::path root, std::string& buffer); // f�r att v�lja en fil
	bool pick_client(client*& buffer); // f�r att v�lja en fil
};