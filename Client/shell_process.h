#pragma once
#include "precompile.h"

struct properties {
	std::string application;
	std::string working_directory;
	DWORD startup;
	DWORD flags;
	WORD visiblilty;
	bool handle_inherit;
};

class shell_process
{
public:

	shell_process() {}

	shell_process(properties prop);

	bool open();

	void ctrl_c();

	bool ready();

	void read(std::function<void(std::string)>);

	std::string read_once();

	bool write(std::string);

	bool alive();

	void close();

	~shell_process();

private:
	STARTUPINFOA info_{};
	PROCESS_INFORMATION process_{};
	SECURITY_ATTRIBUTES security_{};
	properties properties_;
	HANDLE input_read;
	HANDLE input_write;
	HANDLE output_read;
	HANDLE output_write;

	void CloseHandel(HANDLE& handle);
};

