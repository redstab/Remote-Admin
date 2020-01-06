#include "precompile.h"
#include "Error.h"

Error::Error(int) // skapa strukturen error fr�n ett WSA error som nydligen intr�ffade, formatera ett meddelande och s�tt error koden
{
	const int error = WSAGetLastError(); // senaste error fr�n systemet som inkluderar wsa errros 
	code = error; // uppdatera koden
	char msg_buf[256]{ '\0' }; // buffer

	FormatMessageA( // formaterar meddelande 
		FORMAT_MESSAGE_FROM_SYSTEM | // specifierar system error
		FORMAT_MESSAGE_IGNORE_INSERTS | // ignorerar printf syntax
		FORMAT_MESSAGE_MAX_WIDTH_MASK, // f�r att ignorera \n
		nullptr,
		error, // error koden
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // system spr�ket
		msg_buf, // vilken buffer man skriver till
		sizeof(msg_buf), // storlek p� buffer
		nullptr // h�r skulle printf str�ngen varit igentilgen
	);

	msg = std::string(msg_buf); // updatera str�ng
}

Error::Error(int error, std::string message) // skapa ett custom error med egna koder och meddelanden
{
	code = error;
	msg = message;
}
