#pragma once
#include "precompile.h"
#include "ui_element.h"

// f�r att ta emot input fr�n anv�ndaren i en specifik del av sk�rmen
class input_box :
	public ui_element
{
public:
	input_box(window& win, point position, int lenght);

	virtual void edit(); // ta input

	//enkapsulerings funktioner
	void set_length(int length);
	int get_length();

	// �rvda fr�n ui_element
	size get_element_size() const;
	void draw_element();
protected:
	std::string content_; // inputen som anv�ndaren har skrivit
	int length_; // l�ngden som man f�r skriva
};

