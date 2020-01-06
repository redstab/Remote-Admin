#pragma once
#include "precompile.h"
#include "ui_element.h"
// Simpel UI element som används för att skriva ut en text på en position
class text :
	public ui_element
{
public:
	text(const window& win, const std::string& text, point position);

	//Enkapsulerings metoder
	void set_text(const std::string& text);
	std::string get_text() const;

	// ärvda från ui_element
	size get_element_size() const;
	void draw_element(); // skriv ut text

protected:
	std::string text_;
};

