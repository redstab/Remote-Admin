#include "precompile.h"
#include "tcp_client.h"

tcp_client::tcp_client(std::string ip, int port)
{
	if (is_ip(ip)) { // om str�ngen ip �r en ip address
		ip_ = ip;
	}
	else { // annars s� kanske den �r en dns
		std::string dns = dns2string(ip);
		if (dns != ip) { // om den har konverterat 
			ip_ = dns;
		}
		else {
			throw std::exception("invalid ip format, it was neither an ip or a domain");
		}
	}
	port_ = port;
}

bool tcp_client::connect()
{
	server_socket = socket(AF_INET, SOCK_STREAM, 0); // skapa socket
	Error socket_error = server_socket;
	if (socket_error) {
		return false;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port_); // s�tt port
	inet_pton(AF_INET, ip_.c_str(), &hint.sin_addr); // konvertera ip till sockaddr

	Error connect_error = ::connect(server_socket, (sockaddr*)&hint, sizeof(hint)); // f�rs�k ansluta
	if (connect_error)
	{
		closesocket(server_socket); // st�ng socket mo misslyckad anslutning
		return false;
	}

	alive = true;
	return true;

}

void tcp_client::paket_loop()
{
	while (alive) {
		if (readable(server_socket)) { // om servern har skickat n�got
			packet new_packet = receive_packet();
			if (!new_packet.id.empty() && !new_packet.data.empty()) { // om paketet inte �r tomt
				std::cout << "recv()[" << new_packet.id << "|" << new_packet.data << "] - " << Error(0) << std::endl;
				handle_packet(new_packet); // hantera beroende p� hash tabeller
			}
		}
	}
}

bool tcp_client::is_ip(std::string victim)
{
	// ip best�r alltid av minst 4 nummer och 3 punkter
	int dot_count = std::count(victim.begin(), victim.end(), '.');
	int number = std::count_if(victim.begin(), victim.end(), isdigit);
	return dot_count == 3 && number >= 4;
}

std::string tcp_client::dns2string(std::string dns)
{
	// F�r att konvertera dns(google.com) till en ip address (129.291.21.1)
	// anta wsa �r startad
	addrinfo* result{};
	addrinfo hints{};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(dns.c_str(), nullptr, &hints, &result) >= 0) {
		char ip[256];
		unsigned long size = 256;
		WSAAddressToStringA(result->ai_addr, result->ai_addrlen, nullptr, ip, &size);
		dns = ip;
	}

	return dns;
}

bool tcp_client::send(message meddelande)
{
	// med meddelande strukturen ex{ data: KS, id: HELLOWORLD } skapas meddelande buffer som ser ungef�r ut s� h�r: 000000000000010000000000000002HELLOWORLDKS
	std::string buffer = meddelande.buffer();
	return !Error(::send(server_socket, buffer.c_str(), buffer.length(), 0)); // Kolla efter error och skicka packet
}


packet tcp_client::receive_packet()
{
	packet paket;
	header huvud = receive_header(); // ta emot headern 

	if (huvud.data_size > 0 && huvud.id_size > 0) { // om inte error
		paket.id = receive_bytes(huvud.id_size); // ta emot id
		paket.data = receive_bytes(huvud.data_size); // ta emot data
	}

	return paket;
}

std::string tcp_client::receive_bytes(int size)
{
	std::string data;
	std::vector<char> buffer(size, '\0'); // skrivbar buffer till recv
	int bytes_received = recv(server_socket, &buffer.at(0), size, MSG_WAITALL);// anv�nd MSG_WAITALL f�r att l�ta recv fylla hela buffern buffer
	if (bytes_received == size) { // det finns n�gon upper limit p� typ 999999999 bytes som kan allokeras till en str�ng
		data += std::string(buffer.begin(), buffer.end()); // konvertera buffern till str�ng
	}
	else if (bytes_received <= 0) { // server disconnected 
		alive = false;
		disconnect_function();
	}
	return data;
}

std::string tcp_client::receive(int size)
{
	// Ta emot m�nga/ett tcp paket (och kombinera dem till strukturen packet)
	const int tcp_packet_size = 65536; // max-l�ngden p� datan av ett tcp-packet		
	int tcp_packets = size / tcp_packet_size; // antal tcp packet som m�ste tas emot f�r att uppn� storleken 'size'
	int tcp_remainder_size = size - (tcp_packets * tcp_packet_size); // hur mycket data �r kvar f�r att uppn� storleken 'size'

	std::string data_received; // buffer

	while (tcp_packets-- > 0) { // ta emot alla max l�ngd paket
		data_received += receive_bytes(tcp_packet_size);
	}

	return data_received + receive_bytes(tcp_remainder_size); // ta emot resten och returnera
}


header tcp_client::receive_header()
{
	const int header_size = 32;
	std::string head = receive(header_size); // ta emot 32 bytes av header data

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

bool tcp_client::readable(SOCKET sock, int sec, int milli)
{
	fd_set socket_descriptor;
	FD_ZERO(&socket_descriptor);
	FD_SET(sock, &socket_descriptor);
	timeval timeout{ sec, milli };
	return select(0, &socket_descriptor, nullptr, nullptr, &timeout);
	// select returnerar hur m�nga sockets i file-descriptor settet �r l�sbara, eftersom att fd-settet inneh�ller 1 socket s� returnerar funktionen true om l�sbar och 0 om inte l�sbar
}

void tcp_client::handle_packet(packet paket)
{

	std::string id = paket.id;
	std::string data = paket.data;

	if (response_map.count(id)) { // om paket idet finns i pesponse tabellen
		message msg; // skapa nytt meddelande med tabellv�rdet
		msg.identifier = "response|" + id;
		msg.data = response_map[id](data);
		std::cout << "send()[" << msg.buffer() << "]" << std::endl;
		send(msg); // skicka respons
	}
	else if (action_map.count(id)) { // om pakete idet finns i action tabbellen
		action_map[id](data); // aktivera funktionen f�rbinden med idet 
	}
	else { // annars error
		std::cout << "handle_packet()" << " [ " << paket.id << "|" << paket.data << "]" << " - " << Error(-2, "kan inte hantera paket") << std::endl; 
	}

}
