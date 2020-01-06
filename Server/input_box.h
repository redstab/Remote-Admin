#pragma once
#include "precompile.h"
#include "ui_element.h"

// för att ta emot input från användaren i en specifik del av skärmen
class input_box :
	public ui_element
{
public:
	input_box(window& win, point position, int lenght);

	virtual void edit(); // ta input

	//enkapsulerings funktioner
	void set_length(int length);
	int get_length();

	// ärvda från ui_element
	size get_element_size() const;
	void draw_element();
protected:
	std::string content_; // inputen som användaren har skrivit
	int length_; // längden som man får skriva
};

