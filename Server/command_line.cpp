#include "precompile.h"
#include "command_line.h"

command_line::command_line(window& win, point position, size max_size, std::string prompt, func_map functions) :
	window_log(win, position, max_size, true), // skapa barnet med parametrar 
	prompt_{ prompt }, // sätt promptmeddelande
	external_functions_{ functions }, // sätt funktions kartan
	alive_{ true }
{}

void command_line::prompt() {
	*this << prompt_; // skriv prompt meddelandet
	std::string command = input((data_.length() / max_lenght_) - cursor_); // ta input från antal linjer ner man befinner sig minus vilken plats cursorn har, detta gör så att man alltid befinner sig i fönstret med kordinaterna
	
	auto [function, arguments] = split_input(command);

	*this << command + (command.length() < max_lenght_ ? "\n" : ""); // om man redan är vid slutet av buffer behöver man inte skriva en ny linje
	if (!map_exist_execute(external_functions_, function, arguments) && !map_exist_execute(internal_functions_, function, arguments)) {
		*this << "'" << function << "' is not recognized as a command";
	}
}

bool command_line::alive() // konsolen är vid liv?
{
	return alive_;
}

std::string command_line::input(int y)
{
	std::string output = ""; // buffer för inputen

	int key = 0;
	int count = 0; // för att hålla koll så att vi inte skriver för långt
	int length_ = max_size_.x - prompt_.length(); // längden som man för skriva på. Den är längden av hela konsolen minus prompten "server $ "

	while (key != 13) { // medans key inte är enter

		key = getch();

		if (key >= 32 and key <= 126 and count < length_ and (cursor_ + max_size_.y) >(data_.length() / max_lenght_)) { // if alnum + cannot type if scrolling
			output.push_back(key); // lägg till en karaktär i buffern 
			mvwprintw(derived_, y, position_.x + count + prompt_.length() - 1, std::string(1, key).c_str()); // lägg grafiskt till en karaktär
			++count;
		}

		else if ((key == key == 127 or key == '\b' or key == KEY_BACKSPACE) and count > 0 and (cursor_ + max_size_.y) > (data_.length() / max_lenght_)) { // if backspace + cannot type if scrolling
			mvwprintw(derived_, y, position_.x + prompt_.length() + count - 2, " "); // ta grafiskt bort sista karaktären
			output.pop_back(); // ta bort sista karaktären i buffern
			--count; // minska räknaren
		}

		else if (key == KEY_PPAGE) { // if pageup => scroll up
			scroll(-1);
			draw_element(); // updatera konsolen
		}

		else if (key == KEY_NPAGE) { // if pagedown => scroll down
			scroll(1);
			draw_element(); // uppdatera konsolen
		}

		refresh(); // uppdatera fönstret
		wrefresh(derived_);
	}

	return output;
}

std::vector<std::string> command_line::argument_parser(std::string input)
{
	std::istringstream iss(input);
	return std::vector<std::string>(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
}

bool command_line::map_exist_execute(func_map fs, std::string func, std::string args)
{
	if (fs.count(func) != 0) {
		fs[func](args);
	}
	else {
		return false;
	}
}

std::pair<std::string, std::string> command_line::split_input(std::string input)
{
	if (!input.empty() && input[0] != ' ') {
		auto space = input.find_first_of(' ');
		if (space != std::string::npos) {
			return std::make_pair(input.substr(0, space), input.substr(space));
		}
		else {
			return std::make_pair(input, "");
		}
	}
	else {
		return std::make_pair("error", "spacefirst");
	}
}

void command_line::dö()
{
	int i = 10;
	*this << "shuting down";
	while (i-- > 0) { // woho fancy dots
		Sleep(80);
		*this << ".";
	}
	alive_ = false; // break command loop
}
