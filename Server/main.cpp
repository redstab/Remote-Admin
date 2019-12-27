#include "precompile.h"
#include "tcp_server.h"
#include "curse.h"
#include "input_box.h"
#include "title.h"
#include "command_line.h"
#include "line.h"
#include "window_log.h"
#include "server.h"

void draw_wireframe() {
	mvvline(2, COLS / 2, 0, LINES - 2);
	mvvline(0, 0, 0, LINES);
	mvvline(0, COLS - 1, 0, LINES);
	//mvhline(LINES - 3, 0, 0, COLS / 2);
	mvhline(LINES - 1, 0, 0, COLS);
	mvhline(0, 0, 0, COLS);
	mvhline(2, 1, 0, COLS - 2);
	//mvaddch(LINES - 3, 0, ACS_LTEE);
	mvaddch(LINES - 1, 0, ACS_LLCORNER);
	mvaddch(2, 0, ACS_LTEE);
	mvaddch(2, COLS - 1, ACS_RTEE);
	//mvaddch(LINES - 3, COLS / 2, ACS_RTEE);
	mvaddch(LINES - 1, COLS / 2, ACS_BTEE);
	mvaddch(2, COLS / 2, ACS_TTEE);
	mvaddch(0, 0, ACS_ULCORNER);
	mvaddch(0, COLS - 1, ACS_URCORNER);
	mvaddch(LINES - 1, COLS - 1, ACS_LRCORNER);
}

int main()
{


	/*int counter, counter2 = 0;
	initscr();
	curs_set(0);
	attron(A_UNDERLINE);
	mvprintw(0, 24, "NCURSES ALTCHARSET CHARACTERS\n");
	attroff(A_UNDERLINE);
	for (counter = 43; counter < 256; counter++)
	{
		printw("%4d = ", counter);
		addch(counter | A_ALTCHARSET);
		if (counter2 < 7)
		{
			addch(' ');
			addch(ACS_VLINE);
			printw(" ");
		}
		counter2++;
		if (counter2 > 7)
		{
			addch('\n');
			counter2 = 0;
		}
		switch (counter)
		{
		case 46:
			counter = 47;
			break;
		case 48:
			counter = 95;
			break;
		case 97:
			counter = 101;
			break;
		case 126:
			counter = 127;
			break;
		case 128:
			counter = 160;
			break;
		case 172:
			counter = 173;
		}
	}
	getch();
	endwin();
	return 0;*/

	//std::locale::global(std::locale("sw"));
	//setlocale(LC_CTYPE, "");
	curse c;

	startup_wsa();

	const int padding = 5;

	window win({ COLS - padding * 2,LINES - padding });

	title titel(win, " Remote Administration Tool ", 1);
	line vertical_seperator(win, { win.get_size().x / 2, 3 }, win.get_size().y - 4, orientation::vertical);
	line horizontal_seperator(win, { 1, 2 }, win.get_size().x - 2, orientation::horizontal);

	server main(win, { 0,0 }, 54321, {&titel, &vertical_seperator, &horizontal_seperator});

	main.draw_element();

	win.show_border();

	main.bind();

	main.listen();

	main.startup();

	main.cli_loop();

	//func_map fs = {
	//	{"hello", [&](std::string param) {mvprintw(0,0,param.c_str()); refresh(); }}
	//};

	//command_line cli(win, { 1,3 }, { win.get_size().x / 2 - 1, (win.get_size().y) - 4 }, "server $ ", fs);/*(win, { 1,2 }, { win.get_size().x / 2, (win.get_size().y)-3 });*/

	//window_log log(win, { win.get_size().x / 2 + 1,3 }, { win.get_size().x / 2 - 2, win.get_size().y - 4 }, true);

	//tcp_server s(54321, &log);

	//s.bind();
	//s.listen();

	//std::thread server(&tcp_server::run, s);

	//titel.draw_element();
	//vertical_seperator.draw_element();
	//horizontal_seperator.draw_element();

	//win.show_border();

	//while (cli.alive()) {
	//	cli.prompt();
	//}
	//
	//server.join();

	//startup_wsa();

	//tcp_server main(54321, &std::cout);

	//main.bind();

	//main.listen();

	//main.run();

	//WSAData ws;
	//Error wsa = WSAStartup(MAKEWORD(2, 2), &ws);
	//std::cout << "WSAStartup() - " << wsa << std::endl;
	//if (wsa) {
	//	return -1;
	//}

	//SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	//Error socket_error = listen_socket;
	//std::cout << "Socket() - " << socket_error << std::endl;
	//if (socket_error) {
	//	return -1;
	//}
	//sockaddr_in service;
	//service.sin_family = AF_INET;
	//service.sin_port = htons(54321);
	//service.sin_addr.S_un.S_addr = INADDR_ANY;
	//Error binder = bind(listen_socket, (sockaddr*)&service, sizeof(service));
	//std::cout << "bind() - " << binder << std::endl;
	//if (binder) {
	//	return -1;
	//}

	//Error listen_error = ::listen(listen_socket, SOMAXCONN);
	//std::cout << "listen() - " << listen_error << std::endl;
	//if (listen_error) {
	//	return -1;
	//}

	//std::vector<client> clients;

	//std::deque<packet> packets;

	//while (true) {

	//	if (readable(listen_socket)) { // new client
	//		sockaddr_in socket_address{};
	//		char host[NI_MAXHOST]{};
	//		int address_length = sizeof(socket_address);
	//		SOCKET sock = accept(listen_socket, (sockaddr*)&socket_address, &address_length);
	//		inet_ntop(AF_INET, &socket_address.sin_addr, host, NI_MAXHOST);
	//		client ny_client;
	//		ny_client.ip_address = host;
	//		ny_client.socket_id = sock;
	//		ny_client.name = "temp_" + std::to_string(clients.size());
	//		clients.push_back(ny_client);
	//	}

	//	for (auto cli : clients) {
	//		SOCKET sock = cli.socket_id;
	//		if (readable(sock)) { // new data, must be atleast 16 bytes in length
	//			for (auto pack : packets) {
	//				std::cout << "[ data: " << pack.data.substr(0, 10) << " (10/" << pack.data.length() << ") id: " << pack.id.substr(0, 10) << " (10/" << pack.data.length() << ") ]" << std::endl;
	//			}

	//			std::string data;
	//			std::vector<char> buffer(32, '\0');
	//			if (recv(sock, &buffer.at(0), 32, MSG_PEEK) == 32) {
	//				data += std::string(buffer.begin(), buffer.end());
	//			}
	//			std::cout << "Incoming Header: " << data << std::endl;
	//			std::string head = recv_custom(sock, 32);

	//			if (std::all_of(head.begin(), head.end(), isdigit)) {


	//				int id_size = std::stoi(head.substr(0, 16));
	//				int data_size = std::stoi(head.substr(16, 16));

	//				send(sock, "1", 1, 0); // send header confirmation

	//				std::string id(rec(id_size, sock));

	//				std::string data(rec(data_size, sock));

	//				packet nytt_paket;

	//				nytt_paket.data = data;
	//				nytt_paket.data_size = data_size;
	//				nytt_paket.id = id;
	//				nytt_paket.id_size = id_size;
	//				nytt_paket.owner = &cli;

	//				packets.push_back(nytt_paket);

	//			}
	//			else {
	//				std::cout << "  header(verify) - " << Error(103, "Header syntax mismatch") << " " << head << std::endl;
	//				send(sock, "0", 1, 0);
	//			}
	//		}
	//	}
	//}
}