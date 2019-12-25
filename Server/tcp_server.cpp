#include "precompile.h"
#include "tcp_server.h"

void tcp_server::set_log_stream(std::ostream* stream)
{
	log = stream;
}

std::ostream* tcp_server::get_log_stream()
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
	*log << "socket() - " << socket_error << std::endl;
	if (socket_error) {
		throw std::exception(std::string("socket() - " + socket_error.msg).c_str());
	}

	// Bind socket till att acceptera alla anslutningar på porten listen_port
	sockaddr_in service;
	service.sin_family = AF_INET; // tcp
	service.sin_port = htons(listen_port); // port
	service.sin_addr.S_un.S_addr = INADDR_ANY; // vilka adresser
	Error bind_error = ::bind(listen_socket, (sockaddr*)&service, sizeof(service)); // bind
	*log << "bind() - " << bind_error << std::endl;
	if (bind_error) {
		throw std::exception(std::string("bind() - " + bind_error.msg).c_str());
	}
}

void tcp_server::listen()
{

	// Sätter listen_socket i läge för att lyssna efter nya anslutningar
	Error listen_error = ::listen(listen_socket, SOMAXCONN);
	*log << "listen() - " << listen_error << std::endl;
	if (listen_error) {
		throw std::exception(std::string("listen() - " + listen_error.msg).c_str());
	}

}

void tcp_server::run()
{
	while (true) {
		if (readable(listen_socket)) {
			accept_user();
		}
		else {
			for (auto& klient : clients.get_list()) {
				if (readable(klient.socket_id)) {
					if (!packet_queue.empty()) {
						std::cout << "[ " << klient.socket_id << " data: " << packet_queue.back().data.substr(0, 10) << " (10/" << packet_queue.back().data.length() << ") id: " << packet_queue.back().id.substr(0, 10) << " (10/" << packet_queue.back().data.length() << ") ]" << std::endl;
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
	// med meddelande strukturen ex{ data: KS, id: HELLOWORLD } skapas meddelande buffer som ser ungefär ut så här: 000000000000010000000000000002HELLOWORLDKS
	std::string buffer = meddelande.buffer();
	return !Error(::send(klient.socket_id, buffer.c_str(), buffer.length(), 0)); // Kolla efter error och 
}

packet tcp_server::receive_paket(client& klient)
{
	packet paket;
	header huvud = receive_header(klient); // ta emot headern 

	if (huvud.data_size > 0 && huvud.id_size > 0) {
		::send(klient.socket_id, "1", 1, 0); // signalera att headern blev emottagen och att man ska skicka datan och idet
		paket.data = receive_bytes(klient, huvud.data_size);
		paket.id = receive_bytes(klient, huvud.id_size);
		paket.owner = &klient;
	}
	else {
		::send(klient.socket_id, "0", 1, 0); // signalera att header hade error och att man inte ska skicka datan och idet
	}

	return paket;
}

std::string tcp_server::receive_bytes(client& klient, int size)
{
	std::string data;
	std::vector<char> buffer(size, '\0'); // skrivbar buffer till recv
	int bytes_received = recv(klient.socket_id, &buffer.at(0), size, MSG_WAITALL);// använd MSG_WAITALL för att låta recv fylla hela buffern buffer
	if (bytes_received == size) {
		data += std::string(buffer.begin(), buffer.end()); // konvertera buffern till sträng
	}
	else if (bytes_received <= 0) { // user disconnected 
		*log << "Disconnected: " << klient << std::endl;
		clients.disconnect_client(klient);
	}
	return data;
}

std::string tcp_server::receive(client& klient, int size)
{
	// Ta emot många/ett tcp paket (och kombinera dem till strukturen packet)
	const int tcp_packet_size = 65536; // max-längden på datan av ett tcp-packet		
	int tcp_packets = size / tcp_packet_size; // antal tcp packet som måste tas emot för att uppnå storleken 'size'
	int tcp_remainder_size = size - (tcp_packets * tcp_packet_size); // hur mycket data är kvar för att uppnå storleken 'size'

	std::string data_received; // buffer

	while (tcp_packets-- > 0) { // ta emot alla max längd paket
		data_received += receive_bytes(klient, tcp_packet_size);
	}

	return data_received + receive_bytes(klient, tcp_remainder_size); // ta emot resten och returnera
}


header tcp_server::receive_header(client& klient)
{
	const int header_size = 32;
	std::string head = receive(klient, header_size); // ta emot 32 bytes av header data

	if (std::all_of(head.begin(), head.end(), isdigit) && head != "") { // bekräfta att alla bytes är nummer
		header _header;
		_header.id_size = std::stoi(head.substr(0, header_size / 2)); // 0 --> 16 karaktärer blir konverterade till en sträng
		_header.data_size = std::stoi(head.substr(header_size / 2, header_size / 2)); // 16 --> 32 karaktärer blir konverterade till en sträng
		return _header;
	}
	else { // headern är i fel format så därför returnerar vi error header
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
	// select returnerar hur många sockets i file-descriptor settet är läsbara, eftersom att fd-settet innehåller 1 socket så returnerar funktionen true om läsbar och 0 om inte läsbar

}

void tcp_server::accept_user()
{
	sockaddr_in socket_address{}; // sockaddr buffer
	char host[NI_MAXHOST]{}; // buffer for ip address
	int address_length = sizeof(socket_address);
	SOCKET new_klient = accept(listen_socket, (sockaddr*)&socket_address, &address_length); // acceptera ny klient och samla socket information i socket_address 
	inet_ntop(AF_INET, &socket_address.sin_addr, host, NI_MAXHOST); // socket_address till sträng
	clients.add_client(new_klient, host); // lägg till en ny klient i listan
	*log << "accept() - " << clients.search(std::to_string(new_klient)) << std::endl;
}