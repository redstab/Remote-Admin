#pragma once
#include "precompile.h"


// struktur för att hantera olika error meddelande som kan inträffa i tex winapi
struct Error
{
	Error(int); // för att få tag i senaste error meddelandet
	Error(int, std::string); // för att skapa ett custom error kod och meddelande
	int code{ 0 }; // error kod
	std::string msg{ 0 }; // error förklaring

	bool operator==(int input) { // vid jämnförelse med en error och int
		return code == input;
	}
	operator bool() const { // vid användning av if(Error) 
		return code != ERROR_SUCCESS;
	}
};

inline std::ostream& operator<<(std::ostream& os, const Error& m) // vid utskrivt till en ostream
{
	return os << "[" << m.code << "] " << m.msg;
}
