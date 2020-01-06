#pragma once
#include "precompile.h"
#include "tcp_client.h"
#include "process.h"

/// <summary>
/// Container class f�r att f� tcp_clienten att alltid f�rs�ka ansluta n�r den f�rlorar sin anslutning, detta g�rs via disconnect_function som om initializerar tcp_clienten och anluter igen
/// </summary>
class client
{
public:
	client(std::string ip, int port);

	/// <summary>
	/// Garanterar att anslutning lyckas slutligen.
	/// F�rs�ker att �teransluta till anslutningen �r etablerad 
	/// </summary>
	void connect();

	/// <summary>
	/// Starta paket_loop
	/// </summary>
	void run_packet_handler() { client_implentation.paket_loop(); }

private:

	std::string ip_; // anslutnigs ip
	int port_; // anslutnings port

	tcp_client client_implentation; // client instans som ominitializeras n�r den f�rlorar anslutning

	response_table get_responses();
	action_table get_actions();

	response_table responses_ = get_responses(); // tabell f�r att hantera packet som kr�ver en simpel respons

	action_table actions_ = get_actions(); // tabell f�r att hantera packet som kr�ver ingen eller mer �n en respons

	/// <summary>
	/// Funktionen kallas n�r klienten anslutnings avbryts och n�r den ska skapas
	/// </summary>
	void startup();

};

