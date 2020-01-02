#pragma once
#include "precompile.h"

namespace utility {
	std::pair<std::string, std::string> ArgSplit(std::string args, char delimiter) {

		auto found = args.find_first_of(delimiter); // försök hitta första separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det självstående argumentet
			return { args, "" }; 
		}
	}
}