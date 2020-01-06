#pragma once
#include "precompile.h"
#include "text.h"

//Simpel UI element som hanterar en centerad titel
//ärver från text eftersom att det är en typ av text
class title :
	public text
{
public:
	title(const window& win, const std::string& text, int y_position);

	void set_text(const std::string& txt);

	void draw_element();
};

