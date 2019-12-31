#pragma once
#include "tcp_client.h"
#include "process.h"

/// <summary>
/// Container class för att få tcp_clienten att alltid försöka ansluta när den förlorar sin anslutning, detta görs via disconnect_function som om initializerar tcp_clienten och anluter igen
/// </summary>
class client
{
public:
	client(std::string ip, int port, response_table responses, action_table actions);

	/// <summary>
	/// Garanterar att anslutning lyckas slutligen.
	/// Försöker att återansluta till anslutningen är etablerad 
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
	/// Funktionen kallas när klienten anslutnings avbryts och när den ska skapas
	/// </summary>
	void startup();

};

