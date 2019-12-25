#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <Windows.h>
#include <WinSock2.h>
#include <locale>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <queue>
#pragma comment (lib, "ws2_32.lib")

//#include "../Server/Error.h"
struct Error
{
	Error(int);
	Error(int, std::string);
	int code{ 0 };
	std::string msg{ 0 };

	bool operator==(int input) {
		return code == input;
	}
	operator bool() const {
		return code != ERROR_SUCCESS;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m)
{
	return os << "[" << m.code << "] " << m.msg;
}

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

	int code = error;
	msg = std::string(msg_buf);
}

Error::Error(int error, std::string message)
{
	code = error;
	msg = message;
}


int main()
{
	std::string ipAddress = "127.0.0.1";			// IP Address of the server
	int port = 54321;						// Listening port # on the server

	WSAData ws;
	Error wsa = WSAStartup(MAKEWORD(2, 2), &ws);
	std::cout << "WSAStartup() - " << wsa << std::endl;
	if (wsa) {
		return -1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	Error socket_error = sock;
	std::cout << "Socket() - " << socket_error << std::endl;
	if (socket_error) {
		return -1;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	Error conn = connect(sock, (sockaddr*)&hint, sizeof(hint));
	std::cout << "conn() - " << conn << std::endl;
	if (conn)
	{
		closesocket(sock);
		WSACleanup();
		return -1;
	}

	std::string size;

	while (std::cin >> size) {
		int isize = std::stoi(size);
		std::string a;
		a.append(std::string(16 - size.length(), '0'));
		a.append(size);
		a.append(std::string(16 - size.length(), '0'));
		a.append(size);
		std::cout << a << std::endl;
		std::cin.get();
		send(sock, a.c_str(), 32, 0);
		char response[1];
		recv(sock, response, 1, 0);
		if (response[0] == '1') {
			send(sock, std::string(isize, 'a').c_str(), isize, 0);
			send(sock, std::string(isize, 'b').c_str(), isize, 0);
		}
	}

}