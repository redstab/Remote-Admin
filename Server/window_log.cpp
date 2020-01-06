#include "precompile.h"
#include "window_log.h"

window_log::window_log(window& win, point position, size max_size, bool auto_scroll) :
	auto_scroll_{auto_scroll}, // om man vill auto scrolla
	char_count_{ 0 },
	cursor_{ 0 }, 
	hidden_{false},
	derived_(derwin(window_.get_window(), max_size.y, max_size.x, position.y, position.x)), // skapa ett f�nster barn till ui_elements f�nstret
	max_char_count_{ max_size.x * max_size.y }, // kalkylera maximala karakt�rs antalet p� en "sida"
	max_lenght_{ max_size.x }, // maximala rad l�ngd
	max_size_{ max_size }, // max storlek
	ui_element(win, position)
{}

window_log& window_log::operator<<(std::string input)
{
	append(input); // appenda ny str�ng vid (klass << str�ng;)
	if (!hidden_) {
		draw_element();
	}
	return *this;
}

window_log& window_log::operator++()
{
	scroll(-1); // skrolla ner n�r man kallar klass++;
	return *this;
}

window_log& window_log::operator--()
{
	scroll(1); // skrolla upp n�r man kallar klass++;
	return *this;
}

void window_log::append(std::string input)
{
	for (auto c : input) {
		if (c == '\n') { // om det �r en ny linje s� m�ste vi konvertera den till mellanslag eftersom att vi m�ste h�lla r�kning p� karakt�rer
			int rem = max_lenght_ - data_.length() % max_lenght_; // ta reda p� hur m�nga karkat�rer som �r kvar tills slutet p� linjen eftersom att derived_ �r ett WINDOW* s� kommer den att ha word-wrapping. Allst� m�ste vi endast mellanslag till en linje (diff: max-l�ngd p� linje - skrivna karakr�rer p� linjen)
			data_ += std::string(rem, ' ');
			char_count_ += rem;
		}
		else {
			data_ += c;
			char_count_ += 1;
		}

		if (auto_scroll_ && char_count_ > max_char_count_) { // om autoscroll �r p� och om man har skrivit hela f�nstret s� ska man skrolla ner en linje
			scroll(1);
		}
	}
}

bool window_log::scroll(int sc)
{
	// om l�ngden p� skrivna karakt�rer �r minst lika mycket som max karakt�rerna som visas
	// om antalet karakt�rer tills nya cursors �r mindre �n bufferl�ngden minus en hel sida och en rad
	// om den nya cursorn (cursor_+sc) �r st�rre �n noll

	//med dessa vilkor s�  kan vi s�kert uppdatera cursorn
	if (data_.length() >= max_char_count_ && (cursor_+sc)*max_lenght_ < data_.length() - max_char_count_ + max_lenght_ && (cursor_+sc) >= 0) {
		cursor_ += sc;
		return true;
	}
	else {
		return false;
	}
}

void window_log::clear()
{
	data_.clear();
	cursor_ = 0;
	char_count_ = 0;
}

size window_log::get_element_size() const
{
	return max_size_;
}

void window_log::draw_element()
{
	// map f�r att konvertera svenska chars till curses altchars
	std::unordered_map<char, chtype> swedish = { 
		{'�', 229},
		{'�', 228},
		{'�', 246},
		{'�', 197},
		{'�', 196},
		{'�', 214}
	};

	// extrahera en buffer fr�n antalet karakt�rer vid cursor med storleken max_char_count_ eller en hel sida 
	std::string buffer = data_.substr(cursor_ * max_lenght_, max_char_count_); 
	buffer += std::string(max_char_count_ - buffer.length(), ' '); // padda slutet ifall sista raden inte �r helt utskriven 
	wmove(derived_, 0, 0);
	for (auto chr : buffer) {
		if (strchr("������", chr)) {
			waddch(derived_, swedish[chr]);
		}
		else {
			waddch(derived_, chr);
		}
	}

	// uppdatera f�nster
	refresh();
	wrefresh(window_.get_window());
	wrefresh(derived_);
}

void window_log::resize(size new_size)
{
	wresize(derived_, new_size.y, new_size.x);
	max_size_ = new_size;
	max_lenght_ = max_size_.x;
	max_char_count_ = max_size_.x * max_size_.y;
}

WINDOW* window_log::get_derived()
{
	return derived_;
}
