#pragma once
#include "precompile.h"
#include "process.h"

namespace payload {

	std::string process_execution(std::string program) {
		STARTUPINFOA info = { sizeof(info) };
		PROCESS_INFORMATION process_info;
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_CONSOLE, 0, 0, &info, &process_info))
		{
			return "ERROR" + std::to_string(GetLastError());
		}
		else {
			CloseHandle(process_info.hProcess);
			CloseHandle(process_info.hThread);
			return "SUCCESS";
		}
	}

}