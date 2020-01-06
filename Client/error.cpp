#include "precompile.h"
#include "error.h"
Error::Error(int) // int används som tom parameter för att man ska kunna skriv detta. Error temp = winapifunc(); där winapifunc returnerar int
{
	const int error = WSAGetLastError(); // tar senaste error meddelandet 
	char msg_buf[256]{ '\0' }; // buffer

	FormatMessageA( // formaterar meddelande 
		FORMAT_MESSAGE_FROM_SYSTEM | // specifierar system error
		FORMAT_MESSAGE_IGNORE_INSERTS | // ignorerar printf syntax
		FORMAT_MESSAGE_MAX_WIDTH_MASK, // för att ignorera \n
		nullptr,
		error, // error koden
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // system språket
		msg_buf, // vilken buffer man skriver till
		sizeof(msg_buf), // storlek på buffer
		nullptr // hör skulle printf strängen varit igentilgen
	);
	// initializera värden
	code = error;
	msg = std::string(msg_buf);
}

Error::Error(int error, std::string message) // custom konstruktor
{
	code = error;
	msg = message;
}