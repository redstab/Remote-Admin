#pragma once
#include "precompile.h"

// Alla inställnignar som man kan ändra 
struct console_properties {
	std::string console_application;
	std::string working_directory;
	bool inherit_handle = true;
	bool capture_output = true;
	DWORD console_flags = CREATE_NEW_CONSOLE;
	DWORD startup_flags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	WORD show_console = SW_HIDE;
	DWORD timeout = 50;
};

/// <summary>
/// klass som används vid hantering av olika konsol program
/// </summary>
class console_process
{

public:
	console_process(console_properties);
	~console_process();

	/// <summary>
	/// Få tillgång till pipe handles 
	/// </summary>
	/// <returns>pipe handles i en tuple</returns>
	std::tuple<HANDLE, HANDLE, HANDLE, HANDLE> handles();

	/// <summary>
	/// Skapar pipe handles och startar processen 
	/// </summary>
	/// <returns>Om process skapandet lyckades</returns>
	bool open();
	
	/// /// <summary>
	/// Stäng processen som skapades
	/// </summary>
	/// <returns>Om stängningen lyckades</returns>
	bool close();

	/// <summary>
	/// Skriv till input_write handle för att process ska få inputen
	/// </summary>
	/// <param name="input">det som ska skrivas</param>
	/// <returns>Om skrivningen lyckades</returns>
	bool write(std::string);
	
	/// <summary>
	/// Läs från processen och exekevera lambda funktionen som får outputen
	/// </summary>
	/// <param name="output_handle">den funktion som ska hantera output</param>
	void read(std::function<void(std::string)>);

	/// <summary>
	/// write+read i en funktion
	/// </summary>
	/// <param name="output_handler">som hanterar output</param>
	/// <param name="input">strängen som processen ska få </param>
	void execute(std::string, std::function<void(std::string)>);

	/// <summary>
	/// Om process är vid liv
	/// </summary>
	/// <returns>vid liv?</returns>
	bool alive();

private:
	
	STARTUPINFOA startup_info{};
	PROCESS_INFORMATION process_info{};
	SECURITY_ATTRIBUTES security_attrib{};
	
	HANDLE input_read;
	HANDLE input_write;
	HANDLE output_read;
	HANDLE output_write;

	console_properties process_properties;

	bool open_pipe(HANDLE&, HANDLE&);

	void read_console(std::function<void(std::string)>);

	std::string read_pipe(HANDLE&);

};
