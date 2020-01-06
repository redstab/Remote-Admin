#pragma once
#include "precompile.h"


// struktur f�r att hantera olika error meddelande som kan intr�ffa i tex winapi
struct Error
{
	Error(int); // f�r att f� tag i senaste error meddelandet
	Error(int, std::string); // f�r att skapa ett custom error kod och meddelande
	int code{ 0 }; // error kod
	std::string msg{ 0 }; // error f�rklaring

	bool operator==(int input) { // vid j�mnf�relse med en error och int
		return code == input;
	}
	operator bool() const { // vid anv�ndning av if(Error) 
		return code != ERROR_SUCCESS;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt till en ostream
{
	return os << "[" << m.code << "] " << m.msg;
}
