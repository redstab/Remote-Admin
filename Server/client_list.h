#pragma once
#include "precompile.h"
#include "pod.h"

//lista för lätt hantering av klienter
class client_list
{
public:
	// Lägg till en klient till listan
	void add_client(client);
	void add_client(int, std::string);

	// Ta bort och koppla från en klient på listan
	void disconnect_client(client);
	void disconnect_client(std::string);
	void disconnect_client(int);

	// Sök efter klient på listan
	template<typename T> client* search(T client::* member, T value) {
		// Använd generisk funktion med pekare till statisk member för att kunna söka klienter i listan.
		auto klient = std::find_if(list.begin(), list.end(), [&](client c) {return c.*member == value; });
		if (klient != list.end()) {
			return &*klient;
		}
		return nullptr;
	}

	client* search(std::string);

	// Om klienten fortfarande är ansluten
	bool connected(client);
	bool connected(std::string);
	bool connected(int);

	// returnera listan vid tex iterering av klienter
	std::vector<client>& get_list();

	void set_name(std::string name) { prefix_name = name; }

private:
	std::vector<client> list;
	std::string prefix_name; // prefix namn vid namngivning av klienter tex guest_001 där guest är prefixet
};