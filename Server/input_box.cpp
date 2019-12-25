#include "precompile.h"
#include "input_box.h"

input_box::input_box(window& win, point position, int length) : ui_element(win, position), length_{ length }{}

void input_box::edit()
{
	int key = 0;
	int count = content_.length();
	while (key != 13) {
		key = getch();
		if (key >= 32 and key <= 126 and count < length_) {
			content_ += key;
			mvwprintw(window_.get_window(), position_.y, position_.x + count, std::string(1, key).c_str());
			++count;
		}
		else if ((key == key == 127 or key == '\b' or key == KEY_BACKSPACE) && count > 0) {
			mvwprintw(window_.get_window(), position_.y, position_.x + count - 1, " ");
			content_.erase(count - 1, 1);
			--count;
		}
		refresh();
		wrefresh(window_.get_window());
	}
}

void input_box::set_length(int length)
{
	length_ = length;
}

int input_box::get_length()
{
	return length_;
}

size input_box::get_element_size() const
{
	return size{ length_, 1 };
}

void input_box::draw_element()
{
	mvwprintw(window_.get_window(), position_.y, position_.x, content_.c_str());
}
