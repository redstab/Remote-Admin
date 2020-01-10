#include "precompile.h"
#include "client.h"

std::string ip_address = "127.0.0.1";// IP Address of the server
int port = 54321; // Listening port # on the server

// initializera klient till ip och port
client main_client = client(ip_address, port);

BOOL WINAPI DeconstructorOnExit(DWORD fdwCtrlType)
{
	if (fdwCtrlType == CTRL_SHUTDOWN_EVENT || // vid avst�ngning
		fdwCtrlType == CTRL_LOGOFF_EVENT || // vid utloggning
		fdwCtrlType == CTRL_CLOSE_EVENT) // vid st�ngning
	{
		main_client.~client(); // kalla dekonstructor n�r man st�nger konsolen
		return TRUE;
	}
	else {
		return FALSE;
	}

}

int main()
{
	SetConsoleCtrlHandler(DeconstructorOnExit, TRUE); // skapa en rutinhanterare n�r man st�nger konsolen

	std::locale::global(std::locale("sw")); // s� att vi kan anv�nda ���

	startup_wsa(); // starta winsocket

	main_client.connect(); // anslut

	main_client.run_packet_handler(); // k�r packet hanterar
}
