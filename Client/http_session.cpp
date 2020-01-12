#include "precompile.h"
#include "http_session.h"

http_session::http_session(std::string useragent, std::string url)
{
	session_ = WinHttpOpen(std::wstring(useragent.begin(), useragent.end()).c_str(), 0, 0, 0, 0);
	
	if (session_) {
		connection_ = WinHttpConnect(session_, std::wstring(url.begin(), url.end()).c_str(), 443, 0);
		if (!connection_) {
			throw std::exception("WinHttpConnect()", GetLastError());
		}
	}
	else {
		throw std::exception("WinHttpOpen()", GetLastError());
	}
}

std::string http_session::request(std::string method, std::string params)
{
	std::string response;
	
	//skapa en request med metod och parametrar
	request_ = WinHttpOpenRequest(connection_, std::wstring(method.begin(), method.end()).c_str(), std::wstring(params.begin(), params.end()).c_str(), 0, 0, 0, 0x00800000);
	
	// om man skapningen lyckades och vi kunde skicka f�rfr�gan och ta emot svaret
	if (request_ && WinHttpSendRequest(request_, 0, 0, 0, 0, 0, 0) && WinHttpReceiveResponse(request_, 0)) {

		response = receive();

	}
	else {
		throw std::exception("Request()", GetLastError());
	}
	
	return response;
}

std::string http_session::receive()
{
	std::string response;

	unsigned long query_size{};
	unsigned long downloaded{};

	do {

		query_size = 0;
		downloaded = 0;

		// F� storlek p� paketen
		if (WinHttpQueryDataAvailable(request_, &query_size)) {

			std::vector<char> buffer(query_size + 1, '\0'); // allokera en ny vektor med \0 f�r att vid konvertering till st�ng s� �r buffer null-terminated

			// L�s fr�n HINTERNET till buffern
			if (WinHttpReadData(request_, &buffer.at(0), query_size, &downloaded) && downloaded == query_size) {

				// l�gg till i return-buffern
				response += std::string(buffer.begin(), buffer.end());

			}

		}


	} while (query_size > 0); // medans vi har data att l�sa in

	return response;
}

http_session::~http_session()
{
	// F�r att undvika handle l�ckor
	WinHttpCloseHandle(request_);
	WinHttpCloseHandle(connection_);
	WinHttpCloseHandle(session_);
}

