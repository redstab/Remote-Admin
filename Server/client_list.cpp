#include "precompile.h"
#include "client_list.h"

void client_list::add_client(client c) // f�rdigskapade klienter kan bara l�ggas till i vektorn
{
	list.push_back(c);
}

void client_list::add_client(int sock, std::string ip) // skapa en klient fr�n socket idet och ip addressen och v�lj ett namn fr�n prefixen
{
	client c;
	c.ip_address = ip;
	c.socket_id = sock;
	c.name = prefix_name + "_" + std::to_string(list.size() + 1);
	add_client(c);
}

void client_list::disconnect_client(client c) // st�ng socketen och ta bort klienten fr�n vektorn med erase-remove ideom
{
	if (connected(c)) {
		closesocket(c.socket_id);
		list.erase(std::remove(list.begin(), list.end(), c), list.end()); // erase-remove f�r s�ker bortagning av element
	}
}

void client_list::disconnect_client(std::string in) // s�k efter en viss klient med hj�lp av en str�ng (ip eller namn) och disconnekta den
{
	disconnect_client(*search(in));
}

void client_list::disconnect_client(int n) // s�k efter en viss klient med hj�lp av socket_idet och disconnekta den
{
	disconnect_client(*search(std::to_string(n)));
}

// Formatet p� indata str�ngen best�mer vilken member som ska s�kas efter
client* client_list::search(std::string in)
{
	if (std::all_of(in.begin(), in.end(), isdigit)) { // om indatan �r bara siffror s� �r indatan ett socketid
		return search(&client::socket_id, std::stoi(in)); // s�k d� efter membern socket_id med indatan
	}
	else if (in.size() >= 7 && std::all_of(in.begin(), in.end(), [&](char c) {return isdigit(c) || c == '.'; })) { // om indatan �r 15 bytes l�ng och inneh�ller endast siffor och punkter s� �r det en ip_address
		return search(&client::ip_address, in); // s�k d� efter membern ip_address med indatan
	}
	else {
		return search(&client::name, in); // annars �r det nog troligen ett namn s� d� s�ker vi efter name memebern.
	}
}

// verifera om klienten �r ansluten(om den finns i vektorn s� �r den ansluten)
bool client_list::connected(client c)
{
	return std::count(list.begin(), list.end(), c);
}

bool client_list::connected(std::string s)
{
	return connected(*search(s));
}

bool client_list::connected(int i)
{
	return connected(*search(std::to_string(i)));
}

std::vector<client>& client_list::get_list()
{
	return list;
}
