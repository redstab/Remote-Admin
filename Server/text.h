#pragma once
#include "precompile.h"
#include "ui_element.h"
class text :
	public ui_element
{
public:
	text(const window& win, const std::string& text, point position);

	void set_text(const std::string& text);
	std::string get_text() const;

	size get_element_size() const;
	void draw_element();

protected:
	std::string text_;
};

