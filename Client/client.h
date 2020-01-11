#pragma once
#include "precompile.h"
#include "tcp_client.h"
#include "shell_process.h"
#include "directory.h"
#include "process.h"


/// <summary>
/// Container class för att få tcp_clienten att alltid försöka ansluta när den förlorar sin anslutning, detta görs via disconnect_function som om initializerar tcp_clienten och anluter igen
/// </summary>
class client
{
public:
	client(std::string ip, int port);
	~client() {
		shell.~shell_process();
	}
	/// <summary>
	/// Garanterar att anslutning lyckas slutligen.
	/// Försöker att återansluta till anslutningen är etablerad 
	/// </summary>
	void connect();

	/// <summary>
	/// Starta paket_loop
	/// </summary>
	void run_packet_handler() { client_implementation.paket_loop(); }

private:

	std::string ip_; // anslutnigs ip
	int port_; // anslutnings port

	tcp_client client_implementation; // client instans som ominitializeras när den förlorar anslutning

	response_table get_responses();
	action_table get_actions();

	response_table responses_ = get_responses(); // tabell för att hantera packet som kräver en simpel respons

	action_table actions_ = get_actions(); // tabell för att hantera packet som kräver ingen eller mer än en respons

	shell_process shell;

	/// <summary>
	/// Funktionen kallas när klienten anslutnings avbryts och när den ska skapas
	/// </summary>
	void startup();

};

