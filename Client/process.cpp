#include "precompile.h"
#include "process.h"
#include "error.h"

console_process::console_process(console_properties input_properties)
{
	if (input_properties.console_application.empty() || input_properties.working_directory.empty()) { // om dem båda är tomma
		throw std::invalid_argument("received no executable or working directory"); // kasta undantag
	}

	process_properties = input_properties; // kopiera inställningar
	// sätt några inställningar
	startup_info.dwFlags = process_properties.startup_flags; 
	startup_info.wShowWindow = process_properties.show_console; 

	// initializera security attribut som används vid skapandet av pipes
	security_attrib = { sizeof(SECURITY_ATTRIBUTES), 0, process_properties.inherit_handle };
}

console_process::~console_process()
{
	// stäng alla handles för att undvika läcka
	CloseHandle(process_info.hProcess);
	CloseHandle(process_info.hThread);
	CloseHandle(input_read);
	CloseHandle(input_write);
	CloseHandle(output_read);
	CloseHandle(output_write);

}

std::tuple<HANDLE, HANDLE, HANDLE, HANDLE> console_process::handles() // returnera alla pipe handles
{
	return std::make_tuple(input_read, input_write, output_read, output_write);
}

bool console_process::open()
{
	// skapa pipes för input och output
	open_pipe(input_read, input_write);
	open_pipe(output_read, output_write);

	//specifiera att den skapande process ska skriva till output_write och ta emot input från input_read
	startup_info.hStdOutput = output_write;
	startup_info.hStdError = output_write;
	startup_info.hStdInput = input_read;

	//skapa process
	return CreateProcessA(process_properties.console_application.c_str(), nullptr, 0, 0, process_properties.inherit_handle, process_properties.console_flags, 0, process_properties.working_directory.c_str(), &startup_info, &process_info);
}

bool console_process::close()
{
	return TerminateProcess(process_info.hProcess, 0);
}

bool console_process::write(std::string input)
{
	DWORD bytes_written;
	//skriv till "filen"/pipen input_write som då går till processen
	return WriteFile(input_write, input.c_str(), input.length(), &bytes_written, nullptr);
}

void console_process::read_console(std::function<void(std::string)> output_handler)
{

	// Läs process output medans den har output kvar och att processen är vid liv

	do {
		output_handler(read_pipe(output_read)); // använd output_handler för att hantera output
	} while (WaitForSingleObject(input_read, process_properties.timeout) == WAIT_OBJECT_0 && alive());

}

std::string console_process::read_pipe(HANDLE& pipe)
{
	unsigned long bytes_available; // hur mycker som ska läsas

	std::vector<char> output; // buffer som skrivs till
			
	if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &bytes_available, nullptr) && bytes_available != 0) { // ta reda på hur mycker som ska läsas genom att pika in på pipen och skriva det till bytes_available
		output.resize(bytes_available); // ändra storlek på buffer till antal bytes som ska tas emot
		if (!ReadFile(pipe, &output.at(0), bytes_available, nullptr, nullptr)) { // ta emot bytes och skriv till buffern
			std::cout << "ReadFile() -> " << Error(0) << std::endl; // vid error
		}
	}

	return std::string(output.begin(), output.end()); // skapa sträng av buffer och retunera

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
	write(command); // skriv kommando
	read(output_handler); // läs output
}

bool console_process::alive()
{
	return WaitForSingleObject(process_info.hProcess, process_properties.timeout) == WAIT_TIMEOUT; // om processen är vid liv
}

bool console_process::open_pipe(HANDLE& read, HANDLE& write)
{
	return CreatePipe(&read, &write, &security_attrib, 0); // skapa pipes med input och output | read och write handles
}