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

std::deque<packet>& tcp_server::get_queue()
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

	log->Log<LOG_INFO>(logger() << str_time() << " socket() - " << socket_error.to_string() << "\n");
	if (socket_error) {
		throw std::exception(std::string("socket() - " + socket_error.msg).c_str());
	}

	// Bind socket till att acceptera alla anslutningar p� porten listen_port
	sockaddr_in service;
	service.sin_family = AF_INET; // tcp
	log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " [socket.sin_family -> tcp]\n");
	service.sin_port = htons(listen_port); // port
	log->Log<LOG_VERBOSE>(logger() << str_time() << " [socket.port -> " << listen_port << "]\n");
	service.sin_addr.S_un.S_addr = INADDR_ANY; // bind socket till alla n�tverks interfaces
	log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " [socket.interface -> INADDR_ANY]\n");
	Error bind_error = ::bind(listen_socket, (sockaddr*)&service, sizeof(service)); // bind socket
	log->Log<LOG_INFO>(logger() << str_time() << " bind() - " << bind_error.to_string() << "\n");
	if (bind_error) {
		throw std::exception(std::string("bind() - " + bind_error.msg).c_str());
	}
}

void tcp_server::listen()
{

	// S�tter listen_socket i l�ge f�r att lyssna efter nya anslutningar
	log->Log<LOG_VERBOSE>(logger() << str_time() << " [socket.max_connections -> " << SOMAXCONN<< "]\n");
	Error listen_error = ::listen(listen_socket, SOMAXCONN);
	log->Log<LOG_INFO>(logger() << str_time() << " listen() - " << listen_error.to_string() << "\n");
	if (listen_error) {
		throw std::exception(std::string("listen() - " + listen_error.msg).c_str());
	}

}

void tcp_server::run()
{
	while (alive) {
		if (readable(listen_socket)) { // om det �r en ny klient
			accept_user();
		}
		else {
			for (auto& klient : clients.get_list()) { // g� igenom alla klienter och titta om dem har paket till servern
				if (readable(klient.socket_id)) { // om klienten har paket
					packet paket = receive_paket(klient); // ta emot
					if (paket.data != "" && paket.id != "") { // titta efter error
						packet_queue.push_back(paket); // l�gg till i k�n 
						log->Log<LOG_INFO>(logger() << str_time() << " recv(" << std::to_string(klient.socket_id) << ") - id[" << paket.id.substr(0, 13) << "..], data[" << std::to_string(paket.data.length()) << "]\n"); // logga motag
					}
					else {
						log->Log<LOG_VERBOSE>(logger() << str_time() << " recv(" << std::to_string(klient.socket_id) << ") - id[" << (paket.id.empty() ? "empty" : paket.id.substr(0, 14)) << "], data[" << (paket.id.empty() ? "empty" : std::to_string(paket.data.length())) << "]\n\n");
					}
				}
			}
		}
	}
}

bool tcp_server::send(client klient, message meddelande)
{
	// med meddelande strukturen ex{ data: KS, id: HELLOWORLD } skapas meddelande buffer som ser ungef�r ut s� h�r [id-size][data-size][id][data]: 000000000000010000000000000002HELLOWORLDKS
	std::string buffer = meddelande.buffer();
	Error send_error = ::send(klient.socket_id, buffer.c_str(), buffer.length(), 0);
	log->Log<LOG_INFO>(logger() << str_time() << " send(" << klient.socket_id << ", " << meddelande.identifier << "|" << meddelande.data.substr(0, 10) << ") - " << (send_error != 0 ? "fail" : "success") << "\n");
	return !send_error; // Kolla efter error och skicka packet
}

std::string tcp_server::str_time()
{
	// konvertera tiden till str�ng
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

	if (huvud.data_size > 0 && huvud.id_size > 0) { // om inte error
		paket.id = receive_bytes(klient, huvud.id_size); // ta emot id
		paket.data = receive_bytes(klient, huvud.data_size); // ta emot data
		paket.owner = &klient; // specifiera vem som �ger paketet
	}

	return paket;
}

std::string tcp_server::receive_bytes(client& klient, int size)
{
	std::string data;
	std::vector<char> buffer(size, '\0'); // skrivbar buffer till recv
	int bytes_received = recv(klient.socket_id, &buffer.at(0), size, MSG_WAITALL);// anv�nd MSG_WAITALL f�r att l�ta recv fylla hela buffern buffer
	if (bytes_received == size) { // det finns n�gon upper limit p� typ 999999999 bytes som kan allokeras till en str�ng
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive_bytes(" << size << ") - correct_size\n");
		data += std::string(buffer.begin(), buffer.end()); // konvertera buffern till str�ng
	}
	else if (bytes_received <= 0) { // user disconnected 
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive_bytes(" << size << ") - incorrect size\n");
		if (Error(bytes_received).code != 10038 && !klient.ip_address.empty()) { // error 10038 �r att utf�ra recv p� n�got som inte �r en socket, allts� m�ste detta inneb�ra att vi har kallat recv p� en klient som �r bortagen d�rf�r b�r vi inte g�r n�got
			log->Log<LOG_INFO>(logger() << str_time() << " disconnect(" << klient.socket_id << ") - [" << std::to_string(klient.socket_id) << "|" << klient.ip_address << "]\n");
			clients.disconnect_client(klient);
		}
	}
	return data;
}

std::string tcp_server::receive(client& klient, int size)
{
	// Ta emot m�nga/ett tcp paket (och kombinera dem till strukturen packet)
	const int tcp_packet_size = 65536; // max-l�ngden p� datan av ett tcp-packet		
	int tcp_packets = size / tcp_packet_size; // antal tcp packet som m�ste tas emot f�r att uppn� storleken 'size'
	int tcp_remainder_size = size - (tcp_packets * tcp_packet_size); // hur mycket data �r kvar f�r att uppn� storleken 'size'
	log->Log<LOG_VERBOSE>(logger() << str_time() << " receive(" << klient.socket_id << ", " << size <<  ") - " << tcp_packets << "r" << tcp_remainder_size << "\n");

	std::string data_received; // buffer

	while (tcp_packets-- > 0) { // ta emot alla max l�ngd paket
		data_received += receive_bytes(klient, tcp_packet_size);
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive(" << klient.socket_id << ")[" << tcp_packets << "] - " << Error(0) << "\n");
	}

	return data_received + receive_bytes(klient, tcp_remainder_size); // ta emot resten och returnera
}


header tcp_server::receive_header(client& klient)
{
	const int header_size = 32;
	std::string head = receive(klient, header_size); // ta emot 32 bytes av header data
	log->Log<LOG_VERBOSE>(logger() << str_time() << " recv_header(" << klient.socket_id << ")" << (head.empty() ? " - empty" : ("\n" + str_time() + "  -> " +  head)) << "\n");
	if (std::all_of(head.begin(), head.end(), isdigit) && head != "") { // bekr�fta att alla bytes �r nummer
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive_header(" << klient.socket_id << ") - all digit\n");
		header _header;
		_header.id_size = std::stoi(head.substr(0, header_size / 2)); // 0 --> 16 karakt�rer blir konverterade till en str�ng
		_header.data_size = std::stoi(head.substr(header_size / 2, header_size / 2)); // 16 --> 32 karakt�rer blir konverterade till en str�ng
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive_header(" << klient.socket_id << ").id_size -> " << _header.id_size << "\n");
		log->Log<LOG_SUPER_VERBOSE>(logger() << str_time() << " receive_header(" << klient.socket_id << ").data_size -> " << _header.data_size << "\n");
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
	int readable_sockets = select(0, &socket_descriptor, nullptr, nullptr, &timeout);
	// select returnerar hur m�nga sockets i file-descriptor settet �r l�sbara, eftersom att fd-settet inneh�ller 1 socket s� returnerar funktionen true om l�sbar och 0 om inte l�sbar
	if (readable_sockets >= 1) {
		log->Log<LOG_SUPER_VERBOSE>(logger() << "\n" << str_time() << " select(" << sec << "s, " << milli << "ms) -> true\n");
		return true;
	}
	else {
		return false;
	}

}

void tcp_server::accept_user()
{
	sockaddr_in socket_address{}; // sockaddr buffer
	char host[NI_MAXHOST]{}; // buffer for ip address
	int address_length = sizeof(socket_address);
	SOCKET new_klient = accept(listen_socket, (sockaddr*)&socket_address, &address_length); // acceptera ny klient och samla socket information i socket_address 
	log->Log<LOG_INFO>(logger() << str_time() << " accept(" << int(listen_socket) << ") - " << Error(0).to_string() << "");
	inet_ntop(AF_INET, &socket_address.sin_addr, host, NI_MAXHOST); // socket_address till str�ng
	log->Log<LOG_VERBOSE>(logger() << str_time() << " [accept address -> " << host << "]\n");
	clients.add_client(new_klient, host); // l�gg till en ny klient i listan
}

void tcp_server::delete_packet(packet p)
{
	// ta bort packet fr�n packet_queue
	packet_queue.erase(std::remove(packet_queue.begin(), packet_queue.end(), p), packet_queue.end());
}
