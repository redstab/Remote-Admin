#include "precompile.h"
#include "tcp_server.h"

void tcp_server::set_log_stream(window_log* stream)
{
	log = stream;
}

window_log* tcp_server::get_log_stream()
{
	return log;
}

std::queue<packet>& tcp_server::get_queue()
{
	return packet_queue;
}

client_list& tcp_server::get_clients()
{
	return clients;
}

void tcp_server::bind()
{
	// Skapa socket
	listen_socket = ::socket(AF_INET, SOCK_STREAM, 0);
	
	Error socket_error = listen_socket;


	*log << str_time() << " socket() - " << socket_error.to_string() << "\n";
	if (socket_error) {
		throw std::exception(std::string("socket() - " + socket_error.msg).c_str());
	}

	// Bind socket till att acceptera alla anslutningar p� porten listen_port
	sockaddr_in service;
	service.sin_family = AF_INET; // tcp
	service.sin_port = htons(listen_port); // port
	service.sin_addr.S_un.S_addr = INADDR_ANY; // vilka adresser
	Error bind_error = ::bind(listen_socket, (sockaddr*)&service, sizeof(service)); // bind
	*log << str_time() << " bind() - " << bind_error.to_string() << "\n";
	if (bind_error) {
		throw std::exception(std::string("bind() - " + bind_error.msg).c_str());
	}
}

void tcp_server::listen()
{

	// S�tter listen_socket i l�ge f�r att lyssna efter nya anslutningar
	Error listen_error = ::listen(listen_socket, SOMAXCONN);
	*log << str_time() << " listen() - " << listen_error.to_string() << "\n";
	if (listen_error) {
		throw std::exception(std::string("listen() - " + listen_error.msg).c_str());
	}

}

void tcp_server::run()
{
	while (alive) {
		if (readable(listen_socket)) {
			accept_user();
		}
		else {
			for (auto& klient : clients.get_list()) {
				if (readable(klient.socket_id)) {
					if (!packet_queue.empty()) {
						*log << str_time() << " [ " << std::to_string(klient.socket_id) << " data: " << packet_queue.back().data.substr(0,16) << " id: " << packet_queue.back().id.substr(0, 16) << " ]\n";
					}
					packet paket = receive_paket(klient);
					if (paket.data != "" && paket.id != "") {
						packet_queue.push(paket);
					}
				}
			}
		}
	}
}

bool tcp_server::send(client klient, message meddelande)
{
	// med meddelande strukturen ex{ data: KS, id: HELLOWORLD } skapas meddelande buffer som ser ungef�r ut s� h�r: 000000000000010000000000000002HELLOWORLDKS
	std::string buffer = meddelande.buffer();
	return !Error(::send(klient.socket_id, buffer.c_str(), buffer.length(), 0)); // Kolla efter error och skicka packet
}

std::string tcp_server::str_time()
{
	auto n = std::chrono::system_clock::now();
	auto nc = std::chrono::system_clock::to_time_t(n);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&nc), "[%T]");
	return ss.str();
}

packet tcp_server::receive_paket(client& klient)
{
	packet paket;
	header huvud = receive_header(klient); // ta emot headern 

	if (huvud.data_size > 0 && huvud.id_size > 0) {
		::send(klient.socket_id, "1", 1, 0); // signalera att headern blev emottagen och att man ska skicka datan och idet
		paket.id = receive_bytes(klient, huvud.id_size); // ta emot id
		paket.data = receive_bytes(klient, huvud.data_size); // ta emot data
		paket.owner = &klient; // specifiera vem som �ger paketet
	}
	else {
		//*log << std::to_string(huvud.data_size) << " " << std::to_string(huvud.id_size) << "\n";
		::send(klient.socket_id, "0", 1, 0); // signalera att header hade error och att man inte ska skicka datan och idet, oftast sker detta vid buffer overflow eller n�r klienten disconnectar
	}

	return paket;
}

std::string tcp_server::receive_bytes(client& klient, int size)
{
	std::string data;
	std::vector<char> buffer(size, '\0'); // skrivbar buffer till recv
	int bytes_received = recv(klient.socket_id, &buffer.at(0), size, MSG_WAITALL);// anv�nd MSG_WAITALL f�r att l�ta recv fylla hela buffern buffer
	if (bytes_received == size) {
		data += std::string(buffer.begin(), buffer.end()); // konvertera buffern till str�ng
	}
	else if (bytes_received <= 0) { // user disconnected 
		*log << "disconnect() - " << klient.to_string() << "\n";
		clients.disconnect_client(klient);
	}
	return data;
}

std::string tcp_server::receive(client& klient, int size)
{
	// Ta emot m�nga/ett tcp paket (och kombinera dem till strukturen packet)
	const int tcp_packet_size = 65536; // max-l�ngden p� datan av ett tcp-packet		
	int tcp_packets = size / tcp_packet_size; // antal tcp packet som m�ste tas emot f�r att uppn� storleken 'size'
	int tcp_remainder_size = size - (tcp_packets * tcp_packet_size); // hur mycket data �r kvar f�r att uppn� storleken 'size'

	std::string data_received; // buffer

	while (tcp_packets-- > 0) { // ta emot alla max l�ngd paket
		data_received += receive_bytes(klient, tcp_packet_size);
	}

	return data_received + receive_bytes(klient, tcp_remainder_size); // ta emot resten och returnera
}


header tcp_server::receive_header(client& klient)
{
	const int header_size = 32;
	std::string head = receive(klient, header_size); // ta emot 32 bytes av header data

	if (std::all_of(head.begin(), head.end(), isdigit) && head != "") { // bekr�fta att alla bytes �r nummer
		header _header;
		_header.id_size = std::stoi(head.substr(0, header_size / 2)); // 0 --> 16 karakt�rer blir konverterade till en str�ng
		_header.data_size = std::stoi(head.substr(header_size / 2, header_size / 2)); // 16 --> 32 karakt�rer blir konverterade till en str�ng
		return _header;
	}
	else { // headern �r i fel format s� d�rf�r returnerar vi error header
		header _header;
		_header.data_size = -1;
		_header.id_size = -1;
		return _header;
	}
}

bool tcp_server::readable(SOCKET sock, int sec, int milli)
{
	fd_set socket_descriptor;
	FD_ZERO(&socket_descriptor);
	FD_SET(sock, &socket_descriptor);
	timeval timeout{ sec, milli };
	return select(0, &socket_descriptor, nullptr, nullptr, &timeout);
	// select returnerar hur m�nga sockets i file-descriptor settet �r l�sbara, eftersom att fd-settet inneh�ller 1 socket s� returnerar funktionen true om l�sbar och 0 om inte l�sbar

}

void tcp_server::accept_user()
{
	sockaddr_in socket_address{}; // sockaddr buffer
	char host[NI_MAXHOST]{}; // buffer for ip address
	int address_length = sizeof(socket_address);
	SOCKET new_klient = accept(listen_socket, (sockaddr*)&socket_address, &address_length); // acceptera ny klient och samla socket information i socket_address 
	inet_ntop(AF_INET, &socket_address.sin_addr, host, NI_MAXHOST); // socket_address till str�ng
	clients.add_client(new_klient, host); // l�gg till en ny klient i listan
	*log << str_time() << " accept() - " << Error(0).to_string() << "\n";
}