#pragma once
#include "precompile.h"

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

class console_process
{

public:
	console_process(console_properties);
	~console_process();

	std::tuple<HANDLE, HANDLE, HANDLE, HANDLE> handles();

	bool open();
	bool close();

	bool write(std::string);
	
	void read(std::function<void(std::string)>);

	void execute(std::string, std::function<void(std::string)>);

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
