#pragma once
#include "precompile.h"

//struktur f�r att kunna hantera en klient
struct client {
	int socket_id{ 0 }; // idet p� socketen som klienten har
	std::string ip_address; // ipaddressen p� klienten
	std::string name; // pseudo namnet p� klienten

	std::unordered_map<std::string, std::string> client_information = { // information om klient
		{"Windows Product",""},
		{"Windows Owner",""},
		{"Windows Organization",""},
		{"Windows Architecture",""},
		{"Windows Username",""},
		{"Computer Name",""},
		{"Motherboard Vendor",""},
		{"Motherboard Name",""},
		{"BIOS Vendor",""},
		{"BIOS Version",""},
		{"BIOS Date",""},
		{"Processor Name",""},
		{"Processor Speed",""},
		{"RAM Size",""},
		{"GPU Name",""}
	};

	bool operator==(const client& that) const { // vid j�mnf�relse av olika klienter
		return
			ip_address == that.ip_address &&
			socket_id == that.socket_id &&
			name == that.name;
	}
};

inline std::ostream& operator<<(std::ostream& os, const client& m) // vid utskrift av klient
{
	return os << "[" << m.socket_id << "|" << m.ip_address << "|" << m.name << "]";
}
// struktur vid h�mtning av pseudo "tcp" huvudet
struct header {
	int id_size;
	int data_size;
};
//struktur vid h�mtning av ett paket
struct packet {
	std::string id;
	std::string data;
	client* owner;
};

// struktur vid skickning av en data eller ett meddelande
struct message {
	std::string identifier;
	std::string data;

	std::string buffer() { // anv�nt f�r att seriliza message strukturten till en str�ng f�r att kunna �verf�ra den via send
		std::stringstream ss;
		ss
			<< std::string(16 - identifier.length(), '0') << identifier.length()
			<< std::string(16 - data.length(), '0') << data.length()
			<< identifier
			<< data;

		return ss.str();
	}
};