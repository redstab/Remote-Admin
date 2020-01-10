#include "precompile.h"
#include "client.h"

std::string ip_address = "127.0.0.1";// IP Address of the server
int port = 54321; // Listening port # on the server

// initializera klient till ip och port
client main_client = client(ip_address, port);

BOOL WINAPI DeconstructorOnExit(DWORD fdwCtrlType)
{
	if (fdwCtrlType == CTRL_SHUTDOWN_EVENT || // vid avstängning
		fdwCtrlType == CTRL_LOGOFF_EVENT || // vid utloggning
		fdwCtrlType == CTRL_CLOSE_EVENT) // vid stängning
	{
		main_client.~client(); // kalla dekonstructor när man stänger konsolen
		return TRUE;
	}
	else {
		return FALSE;
	}

}

int main()
{
	SetConsoleCtrlHandler(DeconstructorOnExit, TRUE); // skapa en rutinhanterare när man stänger konsolen

	std::locale::global(std::locale("sw")); // så att vi kan använda åäö

	startup_wsa(); // starta winsocket

	main_client.connect(); // anslut

	main_client.run_packet_handler(); // kör packet hanterar
}
