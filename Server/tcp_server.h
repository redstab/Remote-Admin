#pragma once
#include "precompile.h"
#include "client_list.h"
#include "Error.h"
#include "window_log.h"

// en inline funktion aka copy pasta funktion placerar en kopia av funktion d�r den kallas
inline void startup_wsa() { // anv�nds f�r att initializera windows sockets till verision 2.2
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
	/// <param name="port">port att lyssna p�</param>
	/// <param name="output_stream">ofstream att logga till, kan vara fil, konsol eller pip</param>
	tcp_server(int port, window_log* stream) : listen_port(port), log(stream) {}

	// f�r att enkapsulera logger, detta p� grund av att det �r en os�ker pekare
	void set_log_stream(window_log*);
	window_log* get_log_stream();

	/// <summary>
	/// F� �tkomst till paket k�n f�r manipulering av data
	/// </summary>
	/// <returns>Paketk�n</returns>
	std::queue<packet>& get_queue();

	/// <summary>
	/// F� �tkomst till klient listan
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
	/// Starta packet och klient hanteraren i en separat tr�d
	/// </summary>
	void run();

	/// <summary>
	/// Skicka ett message till en klient
	/// </summary>
	/// <param name="klient">m�lklienten som ska ta emot meddelandet</param>
	/// <param name="meddelande">meddelande att skicka</param>
	/// <returns>True: inga errors, False: send error</returns>
	bool send(client, message);

private:

	client_list clients{}; // lista av klienter

	std::queue<packet> packet_queue{}; // denna �r en k� eftersom att vi inte bryr oss om random access + [FIFO]

	const int listen_port; // denna �r const eftersom att det inte finns n�gon anledning att �ndra p� en lyssningsport 

	SOCKET listen_socket{0}; // tcp socket

	window_log* log; // str�m f�r output av information

	/// <summary>
	/// Ta emot ett psuedo tcp paket 
	/// Private eftersom att man f�r manipulera packet_queuen ist�llet. 
	/// </summary>
	/// <param name="klient">klienten som paketet ska tas emot fr�n</param>
	/// <param name="size">storleken p� paketet</param>
	packet receive_paket(client&);

	/// <summary>
	/// Ta emot 'size' antal bytes fr�n en klient
	/// </summary>
	/// <param name="klient">klient som bytes ska tas emot fr�n</param>
	/// <param name="size">hur m�nga bytes som ska tas emot</param>
	/// <returns>str�ng med datan som togs emot</returns>
	std::string receive_bytes(client&, int);

	/// <summary>
	/// Ta emot en size antal bytes fr�n klienten och returnera det som en str�ng
	/// </summary>
	/// <param name="klient">klienten som paketet ska tas emot fr�n</param>
	/// <param name="size">antal bytes som ska tas emot</param>
	/// <returns>str�ngen som togs emot</returns>
	std::string receive(client&, int);

	/// <summary>
	/// Ta emot psuedo huvudet p� packetet och verifera syntaxen p� den
	/// </summary>
	/// <param name="klient">klienten som huvudet ska tas ifr�n</param>
	/// <returns>header struktur f�r l�tt manipulering av huvudet</returns>
	header receive_header(client&);

	/// <summary>
	/// Verifera om socketen �r l�sbar / om det finns data att ta emot fr�n socketen
	/// </summary>
	/// <param name="sock">socketen som ska testas</param>
	/// <param name="sec">sekunder som man ska v�nta</param>
	/// <param name="milli">millisekunder som man ska v�nta</param>
	/// <returns>om socketen �r l�sbar</returns>
	bool readable(SOCKET, int = 0, int = 0);

	/// <summary>
	/// Acceptera den nydligen anslutna klienten (om den authenticerar sig)(TODO)
	/// </summary>
	void accept_user();

};

