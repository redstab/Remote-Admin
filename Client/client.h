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
	void run() { client_implentation.paket_loop(); }

private:

	std::string ip_;
	int port_;

	tcp_client client_implentation;

	response_table get_responses();
	action_table get_actions();

	response_table responses_ = get_responses();

	action_table actions_ = get_actions();

	/// <summary>
	/// Funktionen kallas n�r klienten anslutnings avbryts och n�r den ska skapas
	/// </summary>
	void startup();

};

