#include "precompile.h"
#include "input_box.h"

input_box::input_box(window& win, point position, int length) : ui_element(win, position), length_{ length }{}

void input_box::edit() // f�r att �ndra input
{
	int key = 0;
	int count = content_.length();
	while (key != 13) { // medans man inte har tryckt enter
		key = getch(); // ta in ny tangent
		if (key >= 32 and key <= 126 and count < length_) { // om man har tyckt en karakt�r och man inte har skrivit f�r l�ngt
			content_ += key; // l�gg till i buffern
			mvwprintw(window_.get_window(), position_.y, position_.x + count, std::string(1, key).c_str()); // skriv ut karakt�ren
			++count; // uppdatera r�knare
		}
		else if ((key == key == 127 or key == '\b' or key == KEY_BACKSPACE) && count > 0) { // om tangenten �r backspace och count �r st�rre �n noll
			mvwprintw(window_.get_window(), position_.y, position_.x + count - 1, " "); // skriv ut mellanslag d�r karakt�ren var
			content_.erase(count - 1, 1); // ta bort fr�n buffern
			--count; // uppdatera r�knare
		}
		// uppdatera sk�rm
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
	return size{ length_, 1 }; // max l�ngden som f�r skrivas och en rad i storlek
}

void input_box::draw_element()
{
	mvwprintw(window_.get_window(), position_.y, position_.x, content_.c_str()); // skriv ut vad som �r i content_ buffern
}
