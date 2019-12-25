#include "precompile.h"
#include "Error.h"

Error::Error(int) // skapa strukturen error från ett WSA error som nydligen inträffade, formatera ett meddelande och sätt error koden
{
	const int error = WSAGetLastError();
	code = error;
	char msg_buf[256]{ '\0' };

	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_MAX_WIDTH_MASK,
		nullptr,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		msg_buf,
		sizeof(msg_buf),
		nullptr
	);

	msg = std::string(msg_buf);
}

Error::Error(int error, std::string message) // skapa ett custom error med egna koder och meddelanden
{
	code = error;
	msg = message;
}
