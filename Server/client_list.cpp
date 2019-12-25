#include "precompile.h"
#include "client_list.h"

void client_list::add_client(client c) // färdigskapade klienter kan bara läggas till i vektorn
{
	list.push_back(c);
}

void client_list::add_client(int sock, std::string ip) // skapa en klient från socket idet och ip addressen och välj ett namn från prefixen
{
	client c;
	c.ip_address = ip;
	c.socket_id = sock;
	c.name = prefix_name + "_" + std::to_string(list.size() + 1);
	add_client(c);
}

void client_list::disconnect_client(client c) // stäng socketen och ta bort klienten från vektorn med erase-remove ideom
{
	closesocket(c.socket_id);
	list.erase(std::remove(list.begin(), list.end(), c), list.end()); // erase-remove för säker bortagning av element
}

void client_list::disconnect_client(std::string in) // sök efter en viss klient med hjälp av en sträng (ip eller namn) och disconnekta den
{
	disconnect_client(search(in));
}

void client_list::disconnect_client(int n) // sök efter en viss klient med hjälp av socket_idet och disconnekta den
{
	disconnect_client(search(std::to_string(n)));
}

// Formatet på indata strängen bestämer vilken member som ska sökas efter
client& client_list::search(std::string in)
{
	if (std::all_of(in.begin(), in.end(), isdigit)) { // om indatan är bara siffror så är indatan ett socketid
		return search(&client::socket_id, std::stoi(in)); // sök då efter membern socket_id med indatan
	}
	else if (in.size() == 15 && std::all_of(in.begin(), in.end(), [&](char c) {return isdigit(c) || c == '.'; })) { // om indatan är 15 bytes lång och innehåller endast siffor och punkter så är det en ip_address
		return search(&client::ip_address, in); // sök då efter membern ip_address med indatan
	}
	else {
		return search(&client::name, in); // annars är det nog troligen ett namn så då söker vi efter name memebern.
	}
}

// verifera om klienten är ansluten(om den finns i vektorn så är den ansluten)
bool client_list::connected(client c) 
{
	return std::count(list.begin(), list.end(), c);
}

bool client_list::connected(std::string s)
{
	return connected(search(s));
}

bool client_list::connected(int i)
{
	return connected(search(std::to_string(i)));
}

std::vector<client>& client_list::get_list()
{
	return list;
}
