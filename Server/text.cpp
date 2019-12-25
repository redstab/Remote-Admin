#include "precompile.h"
#include "text.h"

text::text(const window& win, const std::string& text, point position) : ui_element(win, position), text_{ text }{}

void text::set_text(const std::string& text)
{
	clear_element();
	text_ = text;
}

std::string text::get_text() const
{
	return text_;
}

size text::get_element_size() const
{
	return {static_cast<int>(text_.length()), 1};
}

void text::draw_element()
{
	mvwprintw(window_.get_window(), position_.y, position_.x, text_.c_str());
}
