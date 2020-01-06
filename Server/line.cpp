#include "precompile.h"
#include "line.h"

line::line(const window& win, point begin, int length, orientation rotation) : ui_element(win, begin), length_{ length }, rotation_{ rotation } {}

void line::set_orientation(orientation rotation)
{
	clear_element();
	//kan byta orientation eftersom att normala konsolen så har en "pixel" relatinerna 1:2 mellan längd och höjd. Alltså måste vi dela storlekenm när vi byter från horizontel till vertikalt och dubblera för vice versa
	if (rotation_ == orientation::horizontal and rotation == orientation::vertical) {
		length_ /= 2;
	}
	else if(rotation_ == orientation::vertical and rotation == orientation::horizontal){
		length_ *= 2;
	}

	rotation_ = rotation;
}

orientation line::get_orientation() const
{
	return rotation_;
}

void line::set_length(int length)
{
	length_ = length;
}

int line::get_length() const
{
	return length_;
}

size line::get_element_size() const
{
	//returnerar storlek beroende på orientation
	return ((rotation_ == orientation::vertical) ? size{1, length_} : size{length_, 1});
}

void line::draw_element()
{
	// funktions pekare som väljer print funktionen beroende på orientationen
	// detta funkar eftersom att mvwvline och mvwhline har samma argument
	int (*line_func)(WINDOW*, int, int, chtype, int) = (rotation_ == orientation::vertical) ? mvwvline : mvwhline;
	
	//kalla funktionen som den line_func pekar på 
	line_func(window_.get_window(), position_.y, position_.x, 0, length_);
}
