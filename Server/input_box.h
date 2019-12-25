#pragma once
#include "precompile.h"
#include "ui_element.h"
class input_box :
	public ui_element
{
public:
	input_box(window& win, point position, int lenght);

	virtual void edit();

	void set_length(int length);
	int get_length();

	size get_element_size() const;
	void draw_element();
protected:
	std::string content_;
	int length_;
};

