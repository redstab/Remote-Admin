#include "precompile.h"
#include "command_line.h"

command_line::command_line(window& win, point position, size max_size, std::string prompt, func_map functions) :
	window_log(win, position, max_size, true), // skapa barnet med parametrar 
	prompt_{ prompt }, // s�tt promptmeddelande
	external_functions_{ functions }, // s�tt funktions kartan
	alive_{ true }
{}

void command_line::prompt() {
	*this << prompt_; // skriv prompt meddelandet
	std::string command = input((data_.length() / max_lenght_) - cursor_); // ta input fr�n antal linjer ner man befinner sig minus vilken plats cursorn har, detta g�r s� att man alltid befinner sig i f�nstret med kordinaterna

	auto [function, arguments] = split_input(command); // splita inputen med funktionen och argumenten tex [show, clients] fr�n str�ngen "show clients" d�r den separerar vid mellanslag

	*this << command + (command.length() < max_lenght_ ? "\n" : ""); // om man redan �r vid slutet av buffer beh�ver man inte skriva en ny linje
	if (!map_exist_execute(external_functions_, function, arguments) && !map_exist_execute(internal_functions_, function, arguments)) {
		*this << "'" << function << "' is not recognized as a command" << "\n";
	}
}

bool command_line::alive() // konsolen �r vid liv?
{
	return alive_;
}

std::string command_line::input(int y)
{
	std::string input_ = ""; // buffer f�r inputen

	int key = 0;
	int count = 0; // f�r att h�lla koll s� att vi inte skriver f�r l�ngt
	int length_ = max_size_.x - prompt_.length(); // l�ngden som man f�r skriva p�. Den �r l�ngden av hela konsolen minus prompten tex "server $ "

	while (input_ == "") { // medans outputen �r tom
		while (key != 13) { // medans key inte �r enter

			key = getch();

			if (key >= 32 and key <= 126 and count < length_ and (cursor_ + max_size_.y) >(data_.length() / max_lenght_)) { // if alnum + cannot type if scrolling 
				input_.push_back(key); // l�gg till en karakt�r i buffern 
				mvwprintw(derived_, y, position_.x + count + prompt_.length() - 1, std::string(1, key).c_str()); // l�gg grafiskt till en karakt�r
				++count;
			}

			else if ((key == key == 127 or key == '\b' or key == KEY_BACKSPACE) and count > 0 and (cursor_ + max_size_.y) > (data_.length() / max_lenght_)) { // if backspace + cannot type if scrolling
				mvwprintw(derived_, y, position_.x + prompt_.length() + count - 2, " "); // ta grafiskt bort sista karakt�ren
				input_.pop_back(); // ta bort sista karakt�ren i buffern
				--count; // minska r�knaren
			}

			else if (key == KEY_PPAGE) { // if pageup => scroll up
				scroll(-1);
				draw_element(); // updatera konsolen
			}

			else if (key == KEY_NPAGE) { // if pagedown => scroll down
				scroll(1);
				draw_element(); // uppdatera konsolen
			}

			refresh(); // uppdatera f�nstret
			wrefresh(derived_);
		}
		key = 0;
	}

	return input_;
}

std::vector<std::string> command_line::argument_parser(std::string input)
{
	std::istringstream iss(input);
	return std::vector<std::string>(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());
}

bool command_line::map_exist_execute(func_map fs, std::string func, std::string args)
{
	if (fs.count(func) != 0) { // om nyckeln existerar 
		fs[func](args); // exekvera
		return true;
	}
	else {
		return false; // return false
	}
}

std::pair<std::string, std::string> command_line::split_input(std::string input)
{
	auto space = input.find_first_of(' '); // f�rs�k hitta f�rsta mellanslaget 
	if (space != std::string::npos) { // om den hittade n�got
		return std::make_pair(input.substr(0, space), input.substr(space + 1)); // splita vid mellanslaget
	}
	else {
		return std::make_pair(input, ""); // annars returnera bara funktion d�r inge argument har hittats
	}
}

void command_line::d�()
{
	int i = 10;
	*this << "shuting down";
	while (i-- > 0) { // woho fancy dots
		Sleep(80);
		*this << ".";
	}
	alive_ = false; // break command loop
}
