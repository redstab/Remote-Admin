#include "precompile.h"
#include "ui_element.h"

ui_element::ui_element(const window& win, point position) : window_{ win }, position_{ position } {}

point ui_element::get_position() const
{
	return position_;
}

window ui_element::get_window() const
{
	return window_;
}

void ui_element::set_position(point position)
{
	position_ = position;
}

void ui_element::set_window(const window& win)
{
	window_ = win;
}

void ui_element::move_element(point position)
{
	clear_element(); // rensa 
	set_position(position); // ny position
	draw_element(); // skriv ut igen
}

void ui_element::clear_element() const
{
	size element_size = get_element_size();

	// loopa genom element
	for (int y = position_.y; y < position_.y + element_size.y; ++y) {
		for (int x = position_.x; x < position_.x + element_size.x; ++x) {
			mvwprintw(window_.get_window(), y, x, " "); // rensa koordinat
			wrefresh(window_.get_window());
		}
	}

}

void ui_element::redraw_element()
{
	clear_element(); // rensa
	draw_element(); // skriv ut
}
