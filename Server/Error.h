#pragma once
#include "precompile.h"

// Error wrapper för socket error
struct Error
{
	Error(int);
	Error(int, std::string);
	int code{0};
	std::string msg{0};

	bool operator==(int input) { // vid jämnförelse
		return code == input;
	}
	operator bool() const{ // vid if(error)
		return code != ERROR_SUCCESS;
	}
	std::string to_string() {
		return "[" + std::to_string(code) + "] " + msg;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt std::cout << error << std::endl;
{
	return os << "[" << m.code<< "] " << m.msg;
}
