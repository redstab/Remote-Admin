#pragma once
#include "precompile.h"
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
