#include "precompile.h"
#include "shell_process.h"

shell_process::shell_process(properties input_properties)
{
	// sätt properties och startup info
	properties_ = input_properties;
	info_.wShowWindow = properties_.visiblilty;
	info_.dwFlags = properties_.flags;
	security_ = { sizeof(SECURITY_ATTRIBUTES), 0, properties_.handle_inherit };
}

bool shell_process::open()
{
	//skapa pipes
	CreatePipe(&input_read, &input_write, &security_, 0);
	CreatePipe(&output_read, &output_write, &security_, 0);

	//sätt process output och input till dem skapade pipes
	info_.hStdOutput = output_write;
	info_.hStdInput = input_read;
	info_.hStdError = output_write;

	//skapa process
	return CreateProcessA(0, (LPSTR)properties_.application.c_str(), 0, 0, properties_.handle_inherit, properties_.startup, 0, properties_.working_directory.c_str(), &info_, &process_); // Vänta tills applicationen är skrivbar
}

void shell_process::ctrl_c()
{
	FreeConsole(); // free from ourselfs
	if (AttachConsole(process_.dwProcessId)) { // attach to cmd
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, process_.dwProcessId); // send ctrl-c to cmd
		FreeConsole(); // free from cmd
		AttachConsole(-1); // attach to parent
	}
}

bool shell_process::ready() {
	return WaitForSingleObject(input_read, 0);
}

void shell_process::read(std::function<void(std::string)> output_func)
{
	while (alive()) {
		DWORD bytes = 0;
		std::vector<char> output;
		if (PeekNamedPipe(output_read, 0, 0, 0, &bytes, 0) && bytes != 0) {
			output.resize(bytes);
			if (!ReadFile(output_read, &output.at(0), bytes, 0, 0)) {
				std::cout << "ReadFile() -> " << GetLastError() << std::endl;
			}
		}
		output_func(std::string(output.begin(), output.end()));

	}
}

std::string shell_process::read_once() {
	DWORD bytes = 0;
	std::vector<char> output;
	if (PeekNamedPipe(output_read, 0, 0, 0, &bytes, 0) && bytes != 0) {
		output.resize(bytes);
		if (!ReadFile(output_read, &output.at(0), bytes, 0, 0)) {
			std::cout << "ReadFile() -> " << GetLastError() << std::endl;
		}
	}
	return std::string(output.begin(), output.end());
}

bool shell_process::write(std::string input)
{
	return WriteFile(input_write, input.c_str(), input.length(), nullptr, nullptr);
}

bool shell_process::alive()
{
	return WaitForSingleObject(process_.hProcess, 50);
}

void shell_process::close()
{
	TerminateProcess(process_.hProcess, 0);
}

shell_process::~shell_process()
{
	// stäng alla handles för att undvika läckor
	close();
	CloseHandel(input_read);
	CloseHandel(input_write);
	CloseHandel(output_read);
	CloseHandel(output_write);
	CloseHandel(process_.hProcess);
	CloseHandel(process_.hThread);
}

void shell_process::CloseHandel(HANDLE& handle)
{
	if (handle == INVALID_HANDLE_VALUE && handle != 0) {
		CloseHandle(handle);
	}
}
