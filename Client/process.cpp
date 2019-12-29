#include "precompile.h"
#include "process.h"
#include "error.h"

console_process::console_process(console_properties input_properties)
{
	if (input_properties.console_application.empty() || input_properties.working_directory.empty()) {
		throw std::invalid_argument("received no executable or working directory");
	}

	process_properties = input_properties;

	startup_info.dwFlags = process_properties.startup_flags;
	startup_info.wShowWindow = process_properties.show_console;


	security_attrib = { sizeof(SECURITY_ATTRIBUTES), 0, process_properties.inherit_handle };
}

console_process::~console_process()
{
	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);
	CloseHandle(input_read);
	CloseHandle(input_write);
	CloseHandle(output_read);
	CloseHandle(output_write);

}

std::tuple<HANDLE, HANDLE, HANDLE, HANDLE> console_process::handles()
{
	return std::make_tuple(input_read, input_write, output_read, output_write);
}

bool console_process::open()
{
	open_pipe(input_read, input_write);
	open_pipe(output_read, output_write);

	startup_info.hStdOutput = output_write;
	startup_info.hStdError = output_write;
	startup_info.hStdInput = input_read;

	return CreateProcessA(process_properties.console_application.c_str(), nullptr, 0, 0, process_properties.inherit_handle, process_properties.console_flags, 0, process_properties.working_directory.c_str(), &startup_info, &process_info);
}

bool console_process::close()
{
	return TerminateProcess(process_info.hProcess, 0);
}

bool console_process::write(std::string input)
{
	DWORD bytes_written;

	return WriteFile(input_write, input.c_str(), input.length(), &bytes_written, nullptr);
}

void console_process::read_console(std::function<void(std::string)> output_handler)
{

	// Capture Command Output

	do {
		output_handler(read_pipe(output_read));
	} while (WaitForSingleObject(input_read, process_properties.timeout) == WAIT_OBJECT_0 && alive());

}

std::string console_process::read_pipe(HANDLE& pipe)
{
	unsigned long bytes_available;

	std::vector<char> output;
			
	if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &bytes_available, nullptr) && bytes_available != 0) {
		output.resize(bytes_available);
		if (!ReadFile(pipe, &output.at(0), bytes_available, nullptr, nullptr)) {
			std::cout << "ReadFile() -> " << Error(GetLastError()) << std::endl;
		}
	}

	return std::string(output.begin(), output.end());

}

void console_process::read(std::function<void(std::string)> output_handler)
{
	// Read command output

	read_console(output_handler);

	// Read path

	read_console(output_handler);

}

void console_process::execute(std::string command, std::function<void(std::string)> output_handler)
{
	write(command);
	read(output_handler);
}

bool console_process::alive()
{
	return WaitForSingleObject(process_info.hProcess, process_properties.timeout) == WAIT_TIMEOUT;
}




bool console_process::open_pipe(HANDLE& read, HANDLE& write)
{
	return CreatePipe(&read, &write, &security_attrib, 0);
}

