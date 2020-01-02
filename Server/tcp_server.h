#pragma once
#include "precompile.h"
#include "client_list.h"
#include "Error.h"
#include "window_log.h"

// en inline funktion aka copy pasta funktion placerar en kopia av funktion där den kallas
inline void startup_wsa() { // används för att initializera windows sockets till verision 2.2
	WSAData ws;
	Error wsa = WSAStartup(MAKEWORD(2, 2), &ws);
	//std::cout << "WSAStartup() - " << wsa << std::endl;
	if (wsa) {
		exit(-1);
	}
}

class tcp_server
{

public:

	/// <summary>
	/// Ny server bunden till porten 'port'
	/// </summary>
	/// <param name="port">port att lyssna på</param>
	/// <param name="output_stream">ofstream att logga till, kan vara fil, konsol eller pip</param>
	tcp_server(int port, window_log* stream) : listen_port(port), log(stream), alive{ true } { clients.set_name("klient"); }

	// för att enkapsulera logger, detta på grund av att det är en osäker pekare
	void set_log_stream(window_log*);
	window_log* get_log_stream();

	/// <summary>
	/// Få åtkomst till paket kön för manipulering av data
	/// </summary>
	/// <returns>Paketkön</returns>
	std::deque<packet>& get_queue();

	/// <summary>
	/// Få åtkomst till klient listan
	/// </summary>
	/// <returns>Klientlistan</returns>
	client_list& get_clients();

	/// <summary>
	/// Binda servern
	/// Kastar undantag om funktionen inte lyckas
	/// </summary>
	void bind();

	/// <summary>
	/// Lyssna efter anslutningar
	/// Kastar undantag om funktionen inte lyckas
	/// </summary>
	void listen();

	/// <summary>
	/// Starta packet och klient hanteraren i en separat tråd
	/// </summary>
	void run();

	/// <summary>
	/// Skicka ett message till en klient
	/// </summary>
	/// <param name="klient">målklienten som ska ta emot meddelandet</param>
	/// <param name="meddelande">meddelande att skicka</param>
	/// <returns>True: inga errors, False: send error</returns>
	bool send(client, message);

protected:

	bool alive;

	client_list clients{}; // lista av klienter

	std::deque<packet> packet_queue{}; // denna är en kö eftersom att vi inte bryr oss om random access + [FIFO]

	const int listen_port; // denna är const eftersom att det inte finns någon anledning att ändra på en lyssningsport 

	SOCKET listen_socket{0}; // tcp socket

	window_log* log; // ström för output av information

	/// <summary>
	/// Får tiden som en sträng
	/// </summary>
	/// <returns>tiden</returns>
	std::string str_time();

	/// <summary>
	/// Ta emot ett psuedo tcp paket 
	/// Private eftersom att man får manipulera packet_queuen istället. 
	/// </summary>
	/// <param name="klient">klienten som paketet ska tas emot från</param>
	/// <param name="size">storleken på paketet</param>
	packet receive_paket(client&);

	/// <summary>
	/// Ta emot 'size' antal bytes från en klient
	/// </summary>
	/// <param name="klient">klient som bytes ska tas emot från</param>
	/// <param name="size">hur många bytes som ska tas emot</param>
	/// <returns>sträng med datan som togs emot</returns>
	std::string receive_bytes(client&, int);

	/// <summary>
	/// Ta emot en size antal bytes från klienten och returnera det som en sträng
	/// </summary>
	/// <param name="klient">klienten som paketet ska tas emot från</param>
	/// <param name="size">antal bytes som ska tas emot</param>
	/// <returns>strängen som togs emot</returns>
	std::string receive(client&, int);

	/// <summary>
	/// Ta emot psuedo huvudet på packetet och verifera syntaxen på den
	/// </summary>
	/// <param name="klient">klienten som huvudet ska tas ifrån</param>
	/// <returns>header struktur för lätt manipulering av huvudet</returns>
	header receive_header(client&);

	/// <summary>
	/// Verifera om socketen är läsbar / om det finns data att ta emot från socketen
	/// </summary>
	/// <param name="sock">socketen som ska testas</param>
	/// <param name="sec">sekunder som man ska vänta</param>
	/// <param name="milli">millisekunder som man ska vänta</param>
	/// <returns>om socketen är läsbar</returns>
	bool readable(SOCKET, int = 0, int = 0);

	/// <summary>
	/// Acceptera den nydligen anslutna klienten (om den authenticerar sig)(TODO)
	/// </summary>
	void accept_user();

	/// <summary>
	/// Ta bort ett packet ur kön
	/// </summary>
	/// <param name="paket">paket som ska tas bort</param>
	void delete_packet(packet);

};

