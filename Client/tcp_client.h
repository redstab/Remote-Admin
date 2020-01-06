#pragma once
#include "precompile.h"
#include "error.h"
#include "message.h"
// en inline funktion aka copy pasta funktion placerar en kopia av funktion där den kallas
inline void startup_wsa() { // används för att initializera windows sockets till verision 2.2
	WSAData ws;
	Error wsa = WSAStartup(MAKEWORD(2, 2), &ws);
	//std::cout << "WSAStartup() - " << wsa << std::endl;
	if (wsa) {
		exit(-1);
	}
}

using response_table = std::unordered_map <std::string, std::function<std::string(std::string)>>;
using action_table = std::unordered_map <std::string, std::function<void(std::string)>>;

// klass för att hantera motagning och skickning av data till servern. ansvarar inte för att upperäthålla en ansutning, superklassen är ansvarig för det
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
	/// Försök ansluta med ip och port som angivits 
	/// Undantag om misslyckande
	/// </summary>
	/// <returns>om anslutningen lyckades</returns>
	bool connect();

	/// <summary>
	/// Loop för att ta emot paket
	/// </summary>
	void paket_loop();

	// funktioner för enkapsulering
	void set_ip(std::string ip) { ip_ = ip; }
	void set_port(int port) { port_ = port; }
	std::string get_ip() { return ip_; }
	int get_port() { return port_; }
	void set_disconnect_action(std::function<void()> dc) { disconnect_function = dc; }

	void set_responses(response_table rt) { response_map = rt; }
	void set_actions(action_table at) { action_map = at; }

	/// <summary>
	/// Skicka ett message till servern
	/// </summary>
	/// <param name="meddelande">meddelande att skicka</param>
	/// <returns>True: inga errors, False: send error</returns>
	bool send(message);

	/// <summary>
	/// Ta emot ett psuedo tcp paket 
	/// Private eftersom att man får manipulera packet_queuen istället. 
	/// </summary>
	packet receive_packet();

private:

	bool alive;

	// "hash" tabeller för inkommande paket
	response_table response_map; // för paket som endast kräver en respons, tex server skickar: ge mig ditt användarnamn. klienten skickar användarnamn. utbytes trafik
	action_table action_map; // för paket som kräver en egen funktion. tex servern skickar olika cmd-commandon. vi skapar en cmd prompt och exekeverar kommandon i en loop. utbytes trafik fast med mer än ett paket

	//kommer att användas för att ominitialicera instansen av tcp_client eftersom att state baserade funktioner inte kommer att funka när servern kommer online igen
	std::function<void()> disconnect_function; // kallas när anslutningen upphör

	SOCKET server_socket; // soket som är ansluten till server

	std::string ip_;
	int port_;

	bool is_ip(std::string); // för att verifiera att en sträng är en ip address
	std::string dns2string(std::string); // konvertera mellan dns och ip address. tex example.com -> 123.123.123.123

	/// <summary>
	/// Ta emot 'size' antal bytes från servern
	/// </summary>
	/// <param name="size">hur många bytes som ska tas emot</param>
	/// <returns>sträng med datan som togs emot</returns>
	std::string receive_bytes(int);

	/// <summary>
	/// Ta emot en size antal bytes från servern och returnera det som en sträng
	/// </summary>
	/// <param name="size">antal bytes som ska tas emot</param>
	/// <returns>strängen som togs emot</returns>
	std::string receive(int);

	/// <summary>
	/// Ta emot psuedo huvudet på packetet och verifera syntaxen på den
	/// </summary>
	/// <returns>header struktur för lätt manipulering av huvudet</returns>
	header receive_header();

	/// <summary>
	/// Verifera om socketen är läsbar / om det finns data att ta emot från socketen
	/// </summary>
	/// <param name="sock">socketen som ska testas</param>
	/// <param name="sec">sekunder som man ska vänta</param>
	/// <param name="milli">millisekunder som man ska vänta</param>
	/// <returns>om socketen är läsbar</returns>
	bool readable(SOCKET, int = 0, int = 0);

	/// <summary>
	/// Hanterar paket i antingen response_map eller action_map
	/// </summary>
	/// <param name="paket">paket som ska hanteras</param>
	void handle_packet(packet);
};

