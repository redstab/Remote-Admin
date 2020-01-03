#include "precompile.h"
#include "command_line.h"

command_line::command_line(window& win, point position, size max_size, std::string prompt, func_map functions) :
	window_log(win, position, max_size, true), // skapa barnet med parametrar 
	prompt_{ prompt }, // sätt promptmeddelande
	functions_{ functions }, // sätt funktions kartan
	alive_{ true }
{}

void command_line::prompt() {
	*this << prompt_; // skriv prompt meddelandet
	std::string command = input((data_.length() / max_lenght_) - cursor_); // ta input från antal linjer ner man befinner sig minus vilken plats cursorn har, detta gör så att man alltid befinner sig i fönstret med kordinaterna

	auto [function, arguments] = split_input(command); // splita inputen med funktionen och argumenten tex [show, clients] från strängen "show clients" där den separerar vid mellanslag

	*this << command + "\n"; // om man redan är vid slutet av buffer behöver man inte skriva en ny linje
	if (!map_exist_execute(functions_, function, arguments)) {
		*this << "'" << function << "' is not recognized as a command" << "\n";
	}
}

bool command_line::alive() // konsolen är vid liv?
{
	return alive_;
}

std::string command_line::input_str()
{
	std::string old_prompt = prompt_;
	prompt_ = "";
	std::string command = input((data_.length() / max_lenght_) - cursor_);
	*this << command;
	prompt_ = old_prompt;
	return command;
}

std::string command_line::input(int y)
{
	std::string input_ = ""; // buffer för inputen
	int key = 0;
	int count = 0; // för att hålla koll så att vi inte skriver för mycket
	int line_length = max_size_.x; // max längden per linje 
	int remaining_chars = max_char_count_ - (y * line_length) - prompt_.length();
	int line_count = prompt_.length(); // för att hålla koll så att vi uppdaterar y när man har skrivit hela raden, startar på prompt_.length för att kompensera för prompt meddelandet
	while (input_ == "") { // medans outputen är tom
		while (key != 13) { // medans key inte är enter

			key = getch();

			if (key >= 32 and key <= 126 and count < remaining_chars and (cursor_ + max_size_.y) >(data_.length() / max_lenght_)) { // if alnum + cannot type if scrolling 
				input_.push_back(key); // lägg till en karaktär i buffern 
				mvwprintw(derived_, y, position_.x + line_count - 1, std::string(1, key).c_str()); // lägg grafiskt till en karaktär
				++count;
				++line_count;
				if (line_count >= line_length) { // när man har skrivit hela buffern så måste man uppdatera y värdet
					y += 1;
					line_count = 0;
				}
			}

			else if ((key == key == 127 or key == '\b' or key == KEY_BACKSPACE) and count > 0 and (cursor_ + max_size_.y) > (data_.length() / max_lenght_)) { // if backspace + cannot type if scrolling
				mvwprintw(derived_, y, position_.x + line_count - 2, " "); // ta grafiskt bort sista karaktären
				input_.pop_back(); // ta bort sista karaktären i buffern
				--count;
				--line_count;
				if (line_count < 1 && count > 0) { // när man tar bort sista karaktären på linjen så minskas y för att man flyttas upp
					line_count = line_length;
					y -= 1;
				}
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
			wrefresh(window_.get_window());
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
	auto space = input.find_first_of(' '); // försök hitta första mellanslaget 
	if (space != std::string::npos) { // om den hittade något
		return std::make_pair(input.substr(0, space), input.substr(space + 1)); // splita vid mellanslaget
	}
	else {
		return std::make_pair(input, ""); // annars returnera bara funktion där inge argument har hittats
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
