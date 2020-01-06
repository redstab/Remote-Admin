#pragma once
#include "precompile.h"

//struktur för att kunna hantera en klient
struct client {
	int socket_id{ 0 }; // idet på socketen som klienten har
	std::string ip_address; // ipaddressen på klienten
	std::string name; // pseudo namnet på klienten

	std::unordered_map<std::string, std::string> client_information = { // information om klienten
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

	bool operator==(const client& that) const { // vid jämnförelse av olika klienter
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
// struktur vid hämtning av pseudo "tcp" huvudet
struct header {
	int id_size;
	int data_size;
};
//struktur vid hämtning av ett paket
struct packet {
	std::string id;
	std::string data;
	client* owner;

	bool operator==(const packet& that) const { // vid jämnförelse av olika paket
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

	std::string buffer() { // använt för att seriliza message strukturten till en sträng för att kunna överföra den via send
		std::stringstream ss; // ett problem med stringstream är att den tar hänsyn till locale
		// detta gör att när man skriver råa siffror till streamen så bli dem formaterade med komma 1000 -> 1,000
		// detta pajar grejen med serialazation
		// för att undvika detta så har jag valt att konvertera till sträng innan man skriver till streamen
		ss
			<< std::string(16 - std::to_string(identifier.length()).length(), '0') << std::to_string(identifier.length())
			<< std::string(16 - std::to_string(data.length()).length(), '0') << std::to_string(data.length())
			<< identifier
			<< data;

		return ss.str();
	}
};