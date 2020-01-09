#include "precompile.h"
#include "title.h"


title::title(const window& win, const std::string& text_title, int y_position) : text(win, text_title, { (win.get_size().x - static_cast<int>(text_title.length()))/2, y_position }){}

void title::set_text(const std::string& txt)
{
	clear_element();	
	
	if ((position_.y == 0 or position_.y == window_.get_size().y) && window_.has_border()) {
		window_.show_border();
	}
	else if (!window_.has_border()) {
		window_.hide_border();
	}
	
	text_ = txt;
}

void title::draw_element()
{
	point new_position = { (window_.get_size().x / 2 - static_cast<int>(get_text().length())/2), position_.y }; // kalkylera position för att centerera text
	mvwprintw(window_.get_window(), new_position.y, new_position.x, get_text().c_str()); // skriv text
}
