#pragma once
#include "precompile.h"
// hjälp funktioner till server klassen men har ingen anledning till att vara bunden till instansen 
// detta är eftersom att funktionerna ska kunna användas av olika klasser. dem skulle kunna vara statiska funktioner till klassen men det är typ samma sak som vi gör här
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

	void print_dir(std::vector<std::pair<std::filesystem::path, int>> items, std::string klient, WINDOW* win, int offset, int selected, size max_size) {
		mvwprintw(win, 0, 0, std::string("%-" + std::to_string(max_size.x) + "s").c_str(), (klient + "[" + std::to_string(items.size()) + "] - " + items[1].first.parent_path().string()).c_str());
		for (int i = offset; i < max_size.y - 1 + offset; i++) {
			if (i == selected) {
				wattron(win, A_REVERSE); // om i är selected så byt färg
			}
			if (i < items.size()) {
				if (items[i].second == -9) { // mapp
					mvwprintw(win, i + 1 - offset, 0, std::string("%-" + std::to_string(max_size.x) + "s").c_str(), ("/" + items[i].first.filename().string()).c_str());
				}
				else { // fil
					mvwprintw(win, i + 1 - offset, 0, std::string("%-" + std::to_string(max_size.x - std::to_string(items[i].second).length()) + "s%" + std::to_string(std::to_string(items[i].second).length()) + "d").c_str(), items[i].first.filename().string().substr(0, max_size.x - std::to_string(items[i].second).length()).c_str(), items[i].second);
				}
			}
			else { // om det inte finns tillräckligt många items i vektorn så måste vi padda med mellanslag
				mvwprintw(win, i + 1 - offset, 0, std::string(max_size.x, ' ').c_str());
			}

			if (i == selected) {
				wattroff(win, A_REVERSE);// om i är selected så byt tillbaka färg
			}
			refresh();
			wrefresh(win);
		}
	}

	void print_clients(std::vector<client> items, WINDOW* win, int offset, int selected, size max_size) {
		//centera titeln
		mvwprintw(win, 0, 0, std::string(std::string(max_size.x / 2 - std::string("Connnected Clients").length() / 2, ' ') + "Connnected Clients").c_str());
		int max_ip = std::max_element(items.begin(), items.end(), [&](client a, client b) {
			return a.ip_address.length() < b.ip_address.length();
		})->ip_address.length() + 2; // hitta max längd på ip address

		int max_name = std::max_element(items.begin(), items.end(), [&](client a, client b) {
			return a.name.length() < b.name.length();
		})->name.length() + 2; // hitta max längd på namn

		int max_id = std::to_string(std::max_element(items.begin(), items.end(), [&](client a, client b) {
			return a.socket_id < b.socket_id;
		})->socket_id).length() + 2; // hitta max längd på socket_id

		int top_pad = 3;
		std::string format = "%-*d%-*s%*s"; // socket_id namn         ip

		mvwprintw(win, 1, 0, "%-*s%-*s%*s", max_id, "Id", max_name, "Name", (max_size.x - max_id - max_name), "Address"); // printa header

		mvwhline(win, 2, 0, 0, max_size.x); // skriv linje

		for (int i = offset; i < max_size.y - top_pad + offset; i++) {
			if (i == selected) { 
				wattron(win, A_REVERSE); // om i är selected så byt färg
			}
			if (i < items.size()) {
				//använd printf format för att skapa en tabell 
				mvwprintw(win, i + top_pad - offset, 0, format.c_str(), max_id, items[i].socket_id, max_name, items[i].name.c_str(), (max_size.x - max_id - max_name), items[i].ip_address.c_str());
			}
			else { // om det inte finns tillräckligt många items i vektorn så måste vi padda med mellanslag
				mvwprintw(win, i + top_pad - offset, 0, std::string(max_size.x, ' ').c_str());
			}

			if (i == selected) {
				wattroff(win, A_REVERSE); // om i är selected så byt tillbaka färg
			}
			refresh();
			wrefresh(win);
		}
	}
}