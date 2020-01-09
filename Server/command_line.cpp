#include "precompile.h"
#include "command_line.h"

command_line::command_line(window& win, point position, size max_size, std::string prompt, func_map functions) :
	window_log(win, position, max_size, true), // skapa barnet med parametrar 
	prompt_{ prompt }, // s�tt promptmeddelande
	functions_{ functions }, // s�tt funktions kartan
	alive_{ true }
{}

void command_line::prompt() {
	*this << prompt_; // skriv prompt meddelandet
	std::string command = input((data_.length() / max_lenght_) - cursor_); // ta input fr�n antal linjer ner man befinner sig minus vilken plats cursorn har, detta g�r s� att man alltid befinner sig i f�nstret med kordinaterna

	auto [function, arguments] = split_input(command); // splita inputen med funktionen och argumenten tex [show, clients] fr�n str�ngen "show clients" d�r den separerar vid mellanslag

	*this << command + "\n"; // om man redan �r vid slutet av buffer beh�ver man inte skriva en ny linje
	if (!map_exist_execute(functions_, function, arguments)) {
		*this << "'" << function << "' is not recognized as a command" << "\n";
	}
}

bool command_line::alive() // konsolen �r vid liv?
{
	return alive_;
}

std::string command_line::input_str(bool print_input)
{
	std::string command = input(-1); // -1 f�r att ta input vid sista linjen skriven
	if (print_input) {
		*this << command;
	}
	return command;
}

std::string command_line::input(int y)
{
	std::string input_ = ""; // buffer f�r inputen
	int key = getch(); 
	if (y == -1) { // sista linjen skriven
		y = (data_.length() / max_lenght_) - cursor_; //ta input fr�n antal linjer ner man befinner sig minus vilken plats cursorn har, detta g�r s� att man alltid befinner sig i f�nstret med kordinaterna
	}
	int count = 0; // f�r att h�lla koll s� att vi inte skriver f�r mycket
	int line_length = max_size_.x; // max l�ngden per linje 
	int remaining_chars = max_char_count_ - (y * line_length) - prompt_.length();
	int line_count = data_.length() % line_length; // f�r att h�lla koll s� att vi uppdaterar y n�r man har skrivit hela raden, startar p� prompt_.length f�r att kompensera f�r prompt meddelandet
	while (key != 13 || input_ == "") { // medans key inte �r enter eller buffer �r tom

		if (key >= 32 and key <= 126 and count < remaining_chars and (cursor_ + max_size_.y) > (static_cast<int>(data_.length()) / max_lenght_)) { // if alnum + cannot type if scrolling 
			input_.push_back(key); // l�gg till en karakt�r i buffern 
			mvwprintw(derived_, y, position_.x + line_count - 1, std::string(1, key).c_str()); // l�gg grafiskt till en karakt�r
			++count;
			++line_count;
			if (line_count >= line_length) { // n�r man har skrivit hela buffern s� m�ste man uppdatera y v�rdet
				y += 1;
				line_count = 0;
			}
		}

		else if ((key == 127 or key == '\b' or key == KEY_BACKSPACE) and count > 0 and (cursor_ + max_size_.y) > (static_cast<int>(data_.length()) / max_lenght_)) { // if backspace + cannot type if scrolling
			mvwprintw(derived_, y, position_.x + line_count - 2, " "); // ta grafiskt bort sista karakt�ren
			input_.pop_back(); // ta bort sista karakt�ren i buffern
			--count;
			--line_count;
			if (line_count < 1 && count > 0) { // n�r man tar bort sista karakt�ren p� linjen s� minskas y f�r att man flyttas upp
				line_count = line_length;
				y -= 1;
			}
		}

		else if (key == KEY_PPAGE) { // if pageup => scroll up
			if (scroll(-1)) {
				draw_element(); // updatera konsolen
			}
		}

		else if (key == KEY_NPAGE) { // if pagedown => scroll down
			if (scroll(1)) {
				draw_element(); // uppdatera konsolen
			}
		}

		refresh(); // uppdatera f�nstret
		wrefresh(window_.get_window());
		wrefresh(derived_);

		key = getch();
	}
	key = 0;
	return input_;
}

std::vector<std::string> command_line::argument_parser(std::string input)
{
	std::istringstream iss(input);
	// vektor initilizeringen f�r att spearera input buffern med mellanslag till separata element
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
	while (i-- > 0) { // fancy dots
		Sleep(80);
		*this << ".";
	}
	alive_ = false; // bryt command loop
}
