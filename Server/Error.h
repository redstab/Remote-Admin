#pragma once
#include "precompile.h"

// Error wrapper för system error
struct Error
{
	Error(int); // för att få tag i senaste error meddelandet
	Error(int, std::string); // för att skapa ett custom error kod och meddelande
	int code{0};  // error kod
	std::string msg{0}; // error förklaring

	bool operator==(int input) { // vid jämnförelse
		return code == input;
	}
	operator bool() const{ // vid användning av if(Error) 
		return code != ERROR_SUCCESS;
	}
	std::string to_string() { // för serilzation till en window_log
		return "[" + std::to_string(code) + "] " + msg;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt std::cout << error << std::endl;
{
	return os << "[" << m.code<< "] " << m.msg;
}
