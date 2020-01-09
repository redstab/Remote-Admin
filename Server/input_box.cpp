#include "precompile.h"
#include "input_box.h"

input_box::input_box(window& win, point position, int length) : ui_element(win, position), length_{ length }{}

void input_box::edit() // för att ändra input
{
	int key = 0;
	int count = content_.length();
	while (key != 13) { // medans man inte har tryckt enter
		key = getch(); // ta in ny tangent
		if (key >= 32 and key <= 126 and count < length_) { // om man har tyckt en karaktär och man inte har skrivit för långt
			content_ += key; // lägg till i buffern
			mvwprintw(window_.get_window(), position_.y, position_.x + count, std::string(1, key).c_str()); // skriv ut karaktären
			++count; // uppdatera räknare
		}
		else if ((key == 127 or key == '\b' or key == KEY_BACKSPACE) && count > 0) { // om tangenten är backspace och count är större än noll
			mvwprintw(window_.get_window(), position_.y, position_.x + count - 1, " "); // skriv ut mellanslag där karaktären var
			content_.erase(count - 1, 1); // ta bort från buffern
			--count; // uppdatera räknare
		}
		// uppdatera skärm
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
	return size{ length_, 1 }; // max längden som får skrivas och en rad i storlek
}

void input_box::draw_element()
{
	mvwprintw(window_.get_window(), position_.y, position_.x, content_.c_str()); // skriv ut vad som är i content_ buffern
}
