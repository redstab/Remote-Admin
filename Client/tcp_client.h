#pragma once
#include "precompile.h"
#include "error.h"
#include "message.h"
// en inline funktion aka copy pasta funktion placerar en kopia av funktion d�r den kallas
inline void startup_wsa() { // anv�nds f�r att initializera windows sockets till verision 2.2
	WSAData ws;
	Error wsa = WSAStartup(MAKEWORD(2, 2), &ws);
	//std::cout << "WSAStartup() - " << wsa << std::endl;
	if (wsa) {
		exit(-1);
	}
}

using response_table = std::unordered_map <std::string, std::function<std::string(std::string)>>;
using action_table = std::unordered_map <std::string, std::function<void(std::string)>>;

class tcp_client
{
public:
	/// <summary>
	/// Ny klient som ska anslutas till 'ip' och 'port'
	/// </summary>
	/// <param name="ip">anslutnings address</param>
	/// <param name="port">anslutnings port</param>
	tcp_client(std::string, int);

	/// <summary>
	/// F�rs�k ansluta med ip och port som angivits 
	/// Undantag om misslyckande
	/// </summary>
	/// <returns>om anslutningen lyckades</returns>
	bool connect();

	/// <summary>
	/// Loop f�r att ta emot paket
	/// </summary>
	void paket_loop();

	// funktioner f�r enkapsulering
	void set_ip(std::string ip) { ip_ = ip; }
	void set_port(int port) { port_ = port; }
	std::string get_ip() { return ip_; }
	int get_port() { return port_; }
	void set_disconnect_action(std::function<void()> dc) { disconnect_function = dc; }

	void set_responses(response_table rt) { response_map = rt; }
	void set_actions(action_table at) { action_map = at; }

private:

	bool alive;
	
	// "hash" tabeller f�r inkommande paket
	response_table response_map; // f�r paket som endast kr�ver en respons, tex server skickar: ge mig ditt anv�ndarnamn. klienten skickar anv�ndarnamn. utbytes trafik
	action_table action_map; // f�r paket som kr�ver en egen funktion. tex servern skickar olika cmd-commandon. vi skapar en cmd prompt och exekeverar kommandon i en loop. utbytes trafik fast med mer �n ett paket

	//kommer att anv�ndas f�r att ominitialicera instansen av tcp_client eftersom att state baserade funktioner inte kommer att funka n�r servern kommer online igen
	std::function<void()> disconnect_function; // kallas n�r anslutningen upph�r

	SOCKET server_socket; // soket som �r ansluten till server

	std::string ip_;
	int port_;

	bool is_ip(std::string); // f�r att verifiera att en str�ng �r en ip address
	std::string dns2string(std::string); // konvertera mellan dns och ip address. tex example.com -> 123.123.123.123


	/// <summary>
	/// Skicka ett message till servern
	/// </summary>
	/// <param name="meddelande">meddelande att skicka</param>
	/// <returns>True: inga errors, False: send error</returns>
	bool send(message);

	/// <summary>
	/// Ta emot ett psuedo tcp paket 
	/// Private eftersom att man f�r manipulera packet_queuen ist�llet. 
	/// </summary>
	packet receive_packet();

	/// <summary>
	/// Ta emot 'size' antal bytes fr�n servern
	/// </summary>
	/// <param name="size">hur m�nga bytes som ska tas emot</param>
	/// <returns>str�ng med datan som togs emot</returns>
	std::string receive_bytes(int);

	/// <summary>
	/// Ta emot en size antal bytes fr�n servern och returnera det som en str�ng
	/// </summary>
	/// <param name="size">antal bytes som ska tas emot</param>
	/// <returns>str�ngen som togs emot</returns>
	std::string receive(int);

	/// <summary>
	/// Ta emot psuedo huvudet p� packetet och verifera syntaxen p� den
	/// </summary>
	/// <returns>header struktur f�r l�tt manipulering av huvudet</returns>
	header receive_header();

	/// <summary>
	/// Verifera om socketen �r l�sbar / om det finns data att ta emot fr�n socketen
	/// </summary>
	/// <param name="sock">socketen som ska testas</param>
	/// <param name="sec">sekunder som man ska v�nta</param>
	/// <param name="milli">millisekunder som man ska v�nta</param>
	/// <returns>om socketen �r l�sbar</returns>
	bool readable(SOCKET, int = 0, int = 0);

	/// <summary>
	/// Hanterar paket i antingen response_map eller action_map
	/// </summary>
	/// <param name="paket">paket som ska hanteras</param>
	void handle_packet(packet);
};

