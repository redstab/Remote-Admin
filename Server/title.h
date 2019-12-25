#pragma once
#include "precompile.h"
#include "text.h"
class title :
	public text
{
public:
	title(const window& win, const std::string& text, int y_position);

	void set_text(const std::string& txt);

	void draw_element();
};

