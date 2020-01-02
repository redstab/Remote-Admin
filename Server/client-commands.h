#pragma once
#include "precompile.h"
#include "server.h"
#include "utility.h"
func_map server::get_client_commands()
{
	return {

		{"detach", [&](std::string args) {
			argument_parser(
				[&](std::string value) {
					attached = nullptr; // peka på null itsället för på en klient
					console.set_prompt(default_prompt); // sätt tillbaka prompt meddelandet
					console.set_functions(server_commands); // sätt tillbaka server funktionerna
			}, args, "detach");
		}},

		{"process", [&](std::string args) {
			argument_parser([&](std::string value) {
				message msg;

				// parsa input args för att ta reda på om man vill att processen ska vara synlig eller inte
				auto [arg1, arg2] = utility::ArgSplit(args, ' ');
				if (arg1 == "-v" || arg1 == "-h") { // om användaren specifierade synlighet argumentet
					msg.identifier = "process" + arg1;
					msg.data = arg2;
				}
				else { // om användaren inte gjorde det
					msg.identifier = "process-v";
					msg.data = args;
				}

				if (send(*attached, msg)) { // skicka förfrågan 
					console << "Executing " << msg.data;
					// om förfrågan kom fram, vänta på svar
					packet response = wait_response("response|" + msg.identifier, attached);
					if (response.data == "SUCCESS") { // skapde processen
						console << " -> Success\n";
					}
					else { // kunde inte skapa processen
						console << " -> Error[" << response.data << "]\n";
					}
					delete_packet(response);
				}
				else { // kunde inte skicka meddelande
					console << "Could not send request " << args << "\n";
				}
			}, args, "process");
		}},

		{"help", [&](std::string args) {
			console << "\n";
			for (auto [func, doc] : klient_help) { // skriver ut alla help dokument
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"download", [&](std::string args) {
			argument_parser([&](std::string value) {
				message msg{"download", ""};
				if (!args.empty()) {
					msg.identifier = args;
				}
				else { // Select 
					if (send(*attached, {"select", "C:\\"})) {
						WINDOW* der = console.get_derived();
						size max_size = console.get_element_size();
						console.clear_element();
						refresh();
						wrefresh(window_.get_window());
						wrefresh(der);

						std::vector<std::pair<std::filesystem::path, int>> items;
						packet status = wait_response("dir_status", attached);
						int amount = std::stoi(status.data);
						//console << "amount: " << std::to_string(amount) << "\n";
						for (int i = 0; i < amount; i++)
						{
							packet file = wait_response("filedescription", attached);
							if (file.id == "filedescription" && file.owner == attached) {
								auto [path, size] = utility::ArgSplit(file.data, '|');
								items.push_back({ path, std::stoi(size) });
							}
							delete_packet(file);
						}

						mvwprintw(der, 0, 0, items[1].first.parent_path().string().c_str());

						for (int i = 0; i < max_size.y-1; i++) {
							mvwprintw(der, i+1, 0, items[i].first.filename().string().c_str());
						}
						refresh();
						wrefresh(window_.get_window());
						wrefresh(der);
						getch();
					}
				}
				if (send(*attached, msg)) { // skicka förfrågan 
					console << "Downloading " << msg.data;
					packet response = wait_response("response|" + msg.identifier, attached);
					if (response.data != "FAIL") {
						std::filesystem::path fs = args;
						std::ofstream file(std::filesystem::current_path().string() + "\\" + fs.filename().string(), std::ios::binary);
						file << response.data;
						file.close();
						console << " -> Done\n";
					}
					else {
						console << " -> No such file\n";
					}
					delete_packet(response);
				}
				else { // kunde inte skicka meddelande
					console << "Could not send request " << args << "\n";
				}
			}, args, "download");
		}},

	};
}
