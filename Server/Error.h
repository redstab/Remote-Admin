#pragma once
#include "precompile.h"

// Error wrapper f�r system error
struct Error
{
	Error(int); // f�r att f� tag i senaste error meddelandet
	Error(int, std::string); // f�r att skapa ett custom error kod och meddelande
	int code{0};  // error kod
	std::string msg{0}; // error f�rklaring

	bool operator==(int input) { // vid j�mnf�relse
		return code == input;
	}
	operator bool() const{ // vid anv�ndning av if(Error) 
		return code != ERROR_SUCCESS;
	}
	std::string to_string() { // f�r serilzation till en window_log
		return "[" + std::to_string(code) + "] " + msg;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt std::cout << error << std::endl;
{
	return os << "[" << m.code<< "] " << m.msg;
}
