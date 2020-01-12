#pragma once
#include "precompile.h"
#include <winhttp.h>
#pragma comment (lib, "winhttp.lib")

class http_session
{
public:
	http_session(std::string useragent, std::string url);

	std::string request(std::string method, std::string params);

	~http_session();

private:
	HINTERNET session_{};
	HINTERNET connection_{};
	HINTERNET request_{};

	std::string receive();
};

