#pragma once
#include "precompile.h"
#include "server.h"
#include "utility.h"
func_map server::get_client_commands()
{
	return {

		{"detach", [&](std::string args) {
			argument_parser(
				[&](std::string value) {
					attached = nullptr; // peka p� null its�llet f�r p� en klient
					console.set_prompt(default_prompt); // s�tt tillbaka prompt meddelandet
					console.set_functions(server_commands); // s�tt tillbaka server funktionerna
			}, args, "detach");
		}},

		{"process", [&](std::string args) {
			argument_parser([&](std::string value) {
				message msg;

				// parsa input args f�r att ta reda p� om man vill att processen ska vara synlig eller inte
				auto [arg1, arg2] = utility::ArgSplit(args, ' ');
				
				std::string file;
				if (arg1 == "-p" || arg2 == "-p") { // om anv�ndaren vill v�lja fil
					if (pick_file_attached("C:\\", file)) {
						console << "Picked ->" << file << "\n";
					}
					else {
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

				if (!file.empty()) {
					msg.data = file;
				}
				else {
					msg.data = arg2;
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

		{"help", [&](std::string args) {
			console << "\n";
			for (auto [func, doc] : klient_help) { // skriver ut alla help dokument
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{"download", [&](std::string args) {
			argument_parser([&](std::string value) {
				message msg{"download", ""};
				if (!args.empty() && args == "-p") {  // Select file
					std::string buffer;
					if (pick_file_attached("C:\\", buffer)) {
						console << "Picked ->" << buffer << "\n";
						msg.data = buffer;
					}
					else {
						console << "Aborted\n";
						return;
					}
				}
				else if (!args.empty()) {
					msg.data = args;
				}
				else {
					console << "The syntax of the command is incorrect. try -h\n";
					return;
				}

				if (send(*attached, msg)) { // skicka f�rfr�gan 
					console << "Downloading " << msg.data << "\n";
					packet response = wait_response("response|" + msg.identifier, attached);
					if (response.data != "FAIL") {
						std::filesystem::path fs = msg.data;
						if (!std::filesystem::is_directory("Downloads_" + attached->name) || std::filesystem::exists("Downloads_" + attached->name)) {
							std::filesystem::create_directory("Downloads_" + attached->name);
						}
						std::string filename = std::filesystem::current_path().string() + "\\Downloads_" + attached->name + "\\" + fs.filename().string();
						console << "Wrote -> " << filename << "\n";
						std::ofstream file(filename, std::ios::binary);
						file << response.data;
						file.close();
						console << "Transfer -> Done\n";
					}
					else {
						console << " -> No such file\n";
					}
					delete_packet(response);
				}
				else { // kunde inte skicka meddelande
					console << "Could not send request " << args << "\n";
				}
			}, args, "download");
		}},

	};
}
