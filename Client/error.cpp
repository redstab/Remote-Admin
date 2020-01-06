#include "precompile.h"
#include "error.h"
Error::Error(int) // int anv�nds som tom parameter f�r att man ska kunna skriv detta. Error temp = winapifunc(); d�r winapifunc returnerar int
{
	const int error = WSAGetLastError(); // tar senaste error meddelandet 
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
	// initializera v�rden
	code = error;
	msg = std::string(msg_buf);
}

Error::Error(int error, std::string message) // custom konstruktor
{
	code = error;
	msg = message;
}