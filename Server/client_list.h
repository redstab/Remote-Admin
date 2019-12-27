#pragma once
#include "precompile.h"
#include "pod.h"

//lista f�r l�tt hantering av klienter
class client_list
{
public:
	// L�gg till en klient till listan
	void add_client(client);
	void add_client(int, std::string);

	// Ta bort och koppla fr�n en klient p� listan
	void disconnect_client(client);
	void disconnect_client(std::string);
	void disconnect_client(int);

	// S�k efter klient p� listan
	template<typename T> client& search(T client::*member, T value) { 
		// Anv�nd generisk funktion med pekare till statisk member f�r att kunna s�ka klienter i listan.
		return *std::find_if(list.begin(), list.end(), [&](client c) {return c.*member == value; });
	}

	client& search(std::string);

	// Om klienten fortfarande �r ansluten
	bool connected(client);
	bool connected(std::string);
	bool connected(int);

	// returnera listan vid tex iterering av klienter
	std::vector<client>& get_list();

	void set_name(std::string name) { prefix_name = name; }

private:
	std::vector<client> list;
	std::string prefix_name; // prefix namn vid namngivning av klienter tex guest_001 d�r guest �r prefixet
};