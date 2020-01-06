#include "precompile.h"
#include "window_log.h"

window_log::window_log(window& win, point position, size max_size, bool auto_scroll) :
	auto_scroll_{auto_scroll}, // om man vill auto scrolla
	char_count_{ 0 },
	cursor_{ 0 }, 
	hidden_{false},
	derived_(derwin(window_.get_window(), max_size.y, max_size.x, position.y, position.x)), // skapa ett fönster barn till ui_elements fönstret
	max_char_count_{ max_size.x * max_size.y }, // kalkylera maximala karaktärs antalet på en "sida"
	max_lenght_{ max_size.x }, // maximala rad längd
	max_size_{ max_size }, // max storlek
	ui_element(win, position)
{}

window_log& window_log::operator<<(std::string input)
{
	append(input); // appenda ny sträng vid (klass << sträng;)
	if (!hidden_) {
		draw_element();
	}
	return *this;
}

window_log& window_log::operator++()
{
	scroll(-1); // skrolla ner när man kallar klass++;
	return *this;
}

window_log& window_log::operator--()
{
	scroll(1); // skrolla upp när man kallar klass++;
	return *this;
}

void window_log::append(std::string input)
{
	for (auto c : input) {
		if (c == '\n') { // om det är en ny linje så måste vi konvertera den till mellanslag eftersom att vi måste hålla räkning på karaktärer
			int rem = max_lenght_ - data_.length() % max_lenght_; // ta reda på hur många karkatärer som är kvar tills slutet på linjen eftersom att derived_ är ett WINDOW* så kommer den att ha word-wrapping. Allstå måste vi endast mellanslag till en linje (diff: max-längd på linje - skrivna karakrärer på linjen)
			data_ += std::string(rem, ' ');
			char_count_ += rem;
		}
		else {
			data_ += c;
			char_count_ += 1;
		}

		if (auto_scroll_ && char_count_ > max_char_count_) { // om autoscroll är på och om man har skrivit hela fönstret så ska man skrolla ner en linje
			scroll(1);
		}
	}
}

bool window_log::scroll(int sc)
{
	// om längden på skrivna karaktärer är minst lika mycket som max karaktärerna som visas
	// om antalet karaktärer tills nya cursors är mindre än bufferlängden minus en hel sida och en rad
	// om den nya cursorn (cursor_+sc) är större än noll

	//med dessa vilkor så  kan vi säkert uppdatera cursorn
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
	// map för att konvertera svenska chars till curses altchars
	std::unordered_map<char, chtype> swedish = { 
		{'å', 229},
		{'ä', 228},
		{'ö', 246},
		{'Å', 197},
		{'Ä', 196},
		{'Ö', 214}
	};

	// extrahera en buffer från antalet karaktärer vid cursor med storleken max_char_count_ eller en hel sida 
	std::string buffer = data_.substr(cursor_ * max_lenght_, max_char_count_); 
	buffer += std::string(max_char_count_ - buffer.length(), ' '); // padda slutet ifall sista raden inte är helt utskriven 
	wmove(derived_, 0, 0);
	for (auto chr : buffer) {
		if (strchr("åäöÅÄÖ", chr)) {
			waddch(derived_, swedish[chr]);
		}
		else {
			waddch(derived_, chr);
		}
	}

	// uppdatera fönster
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
