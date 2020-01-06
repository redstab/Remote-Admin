#pragma once
#include "precompile.h"
#include "server.h"
#include "utility.h"
func_map server::get_client_commands()
{
	return {

		{"detach", [&](std::string args) { // återgå till server kommandon
			argument_parser(
				[&](std::string value) {
					attached = nullptr; // peka på null itsället för på en klient
					console.set_prompt(default_prompt); // sätt tillbaka prompt meddelandet
					console.set_functions(server_commands); // sätt tillbaka server funktionerna
			}, args, "detach");
		}},

		{"process", [&](std::string args) { // skapa en process hos klienten
			argument_parser([&](std::string value) {
				message msg;

				// parsa input args för att ta reda på om man vill att processen ska vara synlig eller inte
				auto [arg1, arg2] = utility::ArgSplit(args, ' ');
				
				std::string file;
				if (arg1 == "-p" || arg2 == "-p") { // om användaren vill välja fil
					if (pick_file_attached("C:\\", file)) { // låt användaren välja fil och sätt path till file buffer som passas in som referens i funktionen
						console << "Picked ->" << file << "\n";
					}
					else { // om användaren avbröt filväljning eller klienten förlorade anslutning 
						console << "Aborted\n";
						return;
					}
				}

				if (arg1 == "-v" || arg1 == "-h") { // om användaren specifierade synlighet argumentet
					msg.identifier = "process" + arg1;
					msg.data = arg2;
				}
				else { // om användaren inte gjorde det
					msg.identifier = "process-v";
					msg.data = args;
				}

				if (!file.empty()) { // om man valde en fil så sätt datan till path
					msg.data = file;
				}

				if (send(*attached, msg)) { // skicka förfrågan 
					console << "Executing " << msg.data;
					// om förfrågan kom fram, vänta på svar
					packet response = wait_response("response|" + msg.identifier, attached);
					if (response.data == "SUCCESS") { // skapde processen
						console << " -> Success\n";
					}
					else { // kunde inte skapa processen
						console << " -> Error[" << std::error_code(std::stoi(response.data), std::system_category()).message() << "]\n";
					}
					delete_packet(response);
				}
				else { // kunde inte skicka meddelande
					console << "Could not send request " << args << "\n";
				}
			}, args, "process");
		}},

		{"help", [&](std::string args) { // visa hjälp
			console << "\n";
			for (auto [func, doc] : klient_help) { // skriver ut alla help dokument
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"download", [&](std::string args) { // ladda ner en fil från klienten
			argument_parser([&](std::string value) {
				message msg{"download", ""};
				if (!args.empty() && args == "-p") {  // Om man vill välja en fil att ladda ner
					std::string buffer;
					if (pick_file_attached("C:\\", buffer)) {// låt användaren välja fil och sätt path till buffer som passas in som referens i funktionen
						console << "Picked ->" << buffer << "\n";
						msg.data = buffer;
					}
					else {  // om användaren avbröt filväljning eller klienten förlorade anslutning 
						console << "Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					msg.data = args;
				}
				else { // om det inte fanns något argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				if (send(*attached, msg)) { // skicka förfrågan 
					console << "Downloading " << msg.data << "\n";
					packet response = wait_response("response|" + msg.identifier, attached); // vänta på svar
					if (response.data != "FAIL") { // om svaret inte är FAIL måste filen finnas på klienten
						std::filesystem::path fs = msg.data;

						//skapa en mapp för nedladdningar från klienten
						if (!std::filesystem::is_directory("Downloads_" + attached->name) || std::filesystem::exists("Downloads_" + attached->name)) {
							std::filesystem::create_directory("Downloads_" + attached->name);
						}
						//sätt path filnamnet till desegnerade mappen för klienten
						std::string filename = std::filesystem::current_path().string() + "\\Downloads_" + attached->name + "\\" + fs.filename().string();
						console << "Wrote -> " << filename << "\n";
						std::ofstream file(filename, std::ios::binary); // öppna filen i binärt skrivande
						file << response.data; // skriv fildatan som man tog emot
						file.close(); // stäng filen
						console << "Transfer -> Done\n";
					}
					else {
						console << " -> No such file\n";
					}
					delete_packet(response); // ta bort fil packetet
				}
				else { // kunde inte skicka meddelande
					console << "Could not send request " << args << "\n";
				}
			}, args, "download");
		}},

	};
}
