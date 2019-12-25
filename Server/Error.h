#pragma once
#include "precompile.h"

// Error wrapper f�r socket error
struct Error
{
	Error(int);
	Error(int, std::string);
	int code{0};
	std::string msg{0};

	bool operator==(int input) { // vid j�mnf�relse
		return code == input;
	}
	operator bool() const{ // vid if(error)
		return code != ERROR_SUCCESS;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt std::cout << error << std::endl;
{
	return os << "[" << m.code<< "] " << m.msg;
}
