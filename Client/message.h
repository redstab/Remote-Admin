#pragma once
#include "precompile.h"
// struktur vid h�mtning av pseudo "tcp" huvudet
struct header {
	int id_size;
	int data_size;
};
//struktur vid h�mtning av ett paket
struct packet {
	std::string id;
	std::string data;
};

// struktur vid skickning av en data eller ett meddelande
struct message {
	std::string identifier;
	std::string data;

	std::string buffer() { // anv�nt f�r att seriliza message strukturten till en str�ng f�r att kunna �verf�ra den via send
		std::stringstream ss;
		ss
			<< std::string(16 - std::to_string(identifier.length()).length(), '0') << identifier.length()
			<< std::string(16 - std::to_string(data.length()).length(), '0') << data.length()
			<< identifier
			<< data;

		return ss.str();
	}
};
