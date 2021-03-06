#pragma once
#include "precompile.h"

// Alla inst�llnignar som man kan �ndra 
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
/// klass som anv�nds vid hantering av olika konsol program som kr�ver IO
/// </summary>
class console_process
{

public:
	console_process(console_properties);
	~console_process();

	/// <summary>
	/// F� tillg�ng till pipe handles 
	/// </summary>
	/// <returns>pipe handles i en tuple</returns>
	std::tuple<HANDLE, HANDLE, HANDLE, HANDLE> handles();

	/// <summary>
	/// Skapar pipe handles och startar processen 
	/// </summary>
	/// <returns>Om process skapandet lyckades</returns>
	bool open();
	
	/// /// <summary>
	/// St�ng processen som skapades
	/// </summary>
	/// <returns>Om st�ngningen lyckades</returns>
	bool close();

	/// <summary>
	/// Skriv till input_write handle f�r att process ska f� inputen
	/// </summary>
	/// <param name="input">det som ska skrivas</param>
	/// <returns>Om skrivningen lyckades</returns>
	bool write(std::string);
	
	/// <summary>
	/// L�s fr�n processen och exekevera lambda funktionen som f�r outputen
	/// </summary>
	/// <param name="output_handle">den funktion som ska hantera output</param>
	void read(std::function<void(std::string)>);

	/// <summary>
	/// write+read i en funktion
	/// </summary>
	/// <param name="output_handler">som hanterar output</param>
	/// <param name="input">str�ngen som processen ska f� </param>
	void execute(std::string, std::function<void(std::string)>);

	/// <summary>
	/// Om process �r vid liv
	/// </summary>
	/// <returns>vid liv?</returns>
	bool alive();

private:
	
	STARTUPINFOA startup_info{}; // vid upstart av process
	PROCESS_INFORMATION process_info{}; // under exekvering
	SECURITY_ATTRIBUTES security_attrib{}; // f�r pipe skapande
	
	// pipe handles
	HANDLE input_read;
	HANDLE input_write;
	HANDLE output_read;
	HANDLE output_write;

	//inst�llningar till konsol processen
	console_properties process_properties;

	//skapa en ny pipe
	bool open_pipe(HANDLE&, HANDLE&);

	//l�s fr�n processen och exekvera lamda med resultatet, l�ser mha read_pipe
	void read_console(std::function<void(std::string)>);

	//l�s pipe. tittar hur mycket som kan l�sas, sedan l�ser man exakt s� mycket
	std::string read_pipe(HANDLE&);

};
