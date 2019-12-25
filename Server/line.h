#pragma once
#include "precompile.h"
#include "ui_element.h"

enum class orientation {
	vertical = 0,
	horizontal = 1
};

class line :
	public ui_element
{
public:
	line(const window& win, point begin, int length, orientation rotation);

	virtual void set_orientation(orientation rotation);

	orientation get_orientation() const;

	void set_length(int length);
	int get_length() const;

	size get_element_size() const;
	void draw_element();

protected:
	int length_;
	orientation rotation_;
};

