#pragma once
#include "tcp_client.h"
#include "process.h"

class client
{
public:
	client(std::string ip, int port, response_table responses, action_table actions);

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

	response_table responses_;
	action_table actions_;

	/// <summary>
	/// Funktionen kallas n�r klienten anslutnings avbryts och n�r den ska skapas
	/// </summary>
	void startup();

};

