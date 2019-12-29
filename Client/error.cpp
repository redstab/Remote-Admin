#include "precompile.h"
#include "error.h"
Error::Error(int)
{
	const int error = WSAGetLastError();
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

	code = error;
	msg = std::string(msg_buf);
}

Error::Error(int error, std::string message)
{
	code = error;
	msg = message;
}