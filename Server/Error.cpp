#include "precompile.h"
#include "Error.h"

Error::Error(int) // skapa strukturen error från ett WSA error som nydligen inträffade, formatera ett meddelande och sätt error koden
{
	const int error = WSAGetLastError(); // senaste error från systemet som inkluderar wsa errros 
	code = error; // uppdatera koden
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

	msg = std::string(msg_buf); // updatera sträng
}

Error::Error(int error, std::string message) // skapa ett custom error med egna koder och meddelanden
{
	code = error;
	msg = message;
}
