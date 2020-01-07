#pragma once
#include "ui_element.h"

enum log_level {
	LOG_INFO = 1,			// 001
	LOG_VERBOSE = 2,		// 010
	LOG_SUPER_VERBOSE = 4,	// 100
	LOG_ALL = 7				// 111
};

using logger = std::stringstream;

//klass som används som en log eller en read only "konsol"
class window_log :
	public ui_element
{
public:
	window_log(window& win, point position, size max_size, bool); // konstruktor
	window_log& operator<<(std::string); // vid klass << sträng
	window_log& operator++(); // vid klass++;
	window_log& operator--(); // vid klass--;
	void append(std::string); // för att printa text till konsolen
	template<unsigned int T> void Log(std::stringstream);
	void set_log_level(int level) { log_level_ = level; }
	bool scroll(int); // för att skrolla konsolen
	void clear(); // rensa konsolen
	//virituella funktioner som ärvs av ui_element
	size get_element_size() const; 
	void draw_element();

	void resize(size new_size);
	void hide() { hidden_ = true; }
	void unhide() { hidden_ = false; }

	WINDOW* get_derived(); // få tillgång till underliggande fönster buffern

protected:
	WINDOW* derived_; // barn fönstret till window_.get_window() i ui_element
	size max_size_; // max storlek för barn fönstret
	bool hidden_; // om man inte ska kalla draw_element efter varje << 
	int max_char_count_; // max antal karaktärer i fönstret
	int max_lenght_; // max längd för en rad
	int char_count_; // antal karaktärer i buffern
	int cursor_; // från vilken rad utskrift sker ifrån
	bool auto_scroll_; // om man ska autoskrolla när man har skrivit en hel max_char_count_
	int log_level_; // minimum loglevel som visas av log<log_level>("sfdsf");
	std::string data_; // buffer
};

template<unsigned int T>
inline void window_log::Log(std::stringstream logger)
{
	if (log_level_ & T) {
		*this << logger.str();
	}
}
