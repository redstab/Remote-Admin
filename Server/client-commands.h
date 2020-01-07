#pragma once
#include "precompile.h"
#include "server.h"
#include "utility.h"
func_map server::get_client_commands()
{
	return {

		{"detach", [&](std::string args) { // �terg� till server kommandon
			argument_parser(
				[&](std::string value) {
					attached = nullptr; // peka p� null its�llet f�r p� en klient
					console.set_prompt(default_prompt); // s�tt tillbaka prompt meddelandet
					console.set_functions(utility::add(server_commands, global_commands)); // s�tt tillbaka server funktionerna
			}, args, "detach");
		}},

		{"process", [&](std::string args) { // skapa en process hos klienten
			argument_parser([&](std::string value) {
				message msg;

				// parsa input args f�r att ta reda p� om man vill att processen ska vara synlig eller inte
				auto [arg1, arg2] = utility::ArgSplit(args, ' ');

				std::string file;
				if (arg1 == "-p" || arg2 == "-p") { // om anv�ndaren vill v�lja fil
					if (pick_file("C:\\", file)) { // l�t anv�ndaren v�lja fil och s�tt path till file buffer som passas in som referens i funktionen
						console << "Picked ->" << file << "\n";
					}
					else { // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
						console << "Aborted\n";
						return;
					}
				}

				if (arg1 == "-v" || arg1 == "-h") { // om anv�ndaren specifierade synlighet argumentet
					msg.identifier = "process" + arg1;
					msg.data = arg2;
				}
				else { // om anv�ndaren inte gjorde det
					msg.identifier = "process-v";
					msg.data = args;
				}

				if (!file.empty()) { // om man valde en fil s� s�tt datan till path
					msg.data = file;
				}

				if (send(*attached, msg)) { // skicka f�rfr�gan 
					console << "Executing " << msg.data;
					// om f�rfr�gan kom fram, v�nta p� svar
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

		{"help", [&](std::string args) { // visa hj�lp
			console << "\n";
			for (auto [func, doc] : klient_help) { // skriver ut alla help dokument
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"download", [&](std::string args) { // ladda ner en fil fr�n klienten
			argument_parser([&](std::string value) {
				message msg{"download", ""};

				std::string file;
				std::string folder = "";
				if (!args.empty() && args == "-p") {  // Om man vill v�lja en fil att ladda ner
					if (pick_file("C:\\", file)) {// l�t anv�ndaren v�lja fil och s�tt path till buffer som passas in som referens i funktionen
						console << "File to download -> " << file << "\n\n";
						msg.data = file;
						if (pick_folder_local("C:\\", folder)) {
							console << "Folder to download to -> " << folder << "\n\n";
						}
						else {
							console << "Aborted, File will end up in default folder\n\n";
						}
					}
					else {  // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
						console << "Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					msg.data = args;
				}
				else { // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				if (send(*attached, msg)) { // skicka f�rfr�gan 
					console << "Downloading " << msg.data << "\n\n";
					packet response = wait_response("response|" + msg.identifier, attached); // v�nta p� svar
					if (response.data != "FAIL") { // om svaret inte �r FAIL m�ste filen finnas p� klienten
						std::filesystem::path fs = msg.data;

						//skapa en mapp f�r nedladdningar fr�n klienten
						if (!std::filesystem::is_directory("Downloads_" + attached->name) || std::filesystem::exists("Downloads_" + attached->name)) {
							std::filesystem::create_directory("Downloads_" + attached->name);
						}
						//s�tt path filnamnet till designerade mappen f�r klienten
						std::string filename = std::filesystem::current_path().string() + "\\Downloads_" + attached->name + "\\" + fs.filename().string();

						if (folder != "") {
							filename = folder + "\\" + fs.filename().string();
						}

						console << "Wrote -> " << filename << "\n\n";
						std::ofstream file(filename, std::ios::binary); // �ppna filen i bin�rt skrivande
						file << response.data; // skriv fildatan som man tog emot
						file.close(); // st�ng filen
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

		{"upload", [&](std::string args) { // ladda upp en fil till klienten
			argument_parser([&](std::string value) {
				message msg{"upload", ""};
				std::string upload_file;
				std::string upload_folder;
				if (!args.empty() && args == "-p") {  // Om man vill v�lja en fil att ladda ner
					if (pick_file_local("C:\\", upload_file)) {// l�t anv�ndaren v�lja fil och s�tt path till buffer som passas in som referens i funktionen
						console << "File to upload -> " << upload_file << "\n\n";
						msg.data = upload_file;
						if (pick_folder("C:\\", upload_folder)) {
							console << "Folder to upload to -> " << upload_folder << "\n\n";
							msg.data += "|" + upload_folder;
						}
					}
					else {  // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
						console << "Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					msg.data = args;
				}
				else { // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				if (send(*attached, msg)) { // skicka f�rfr�gan

					console << "Sent request";

					packet response = wait_response("upload_status", attached);

					if (response.data == "Ok") {

						console << " -> ok\n";

						std::ifstream file(upload_file, std::ios::binary); // l�s in filen bin�rt

						send(*attached, {"upload-data", std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>())}); // konvertera fil till str�ng och skicka

						console << "Sent File -> " << upload_folder << "\n";
					}
					else {
						console << " -> bad folder\n";
					}

				}


			}, args, "upload");
		} },

	};
}
