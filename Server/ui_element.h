#pragma once
#include "precompile.h"
#include "point.h"
#include "window.h"
class ui_element
{
public:
	ui_element(const window& win, point position);

	point get_position() const;
	window get_window() const;

	void set_position(point position);
	void set_window(const window& win);

	void move_element(point position);
	void clear_element() const;
	void redraw_element();
	
	virtual void draw_element() = 0;
	virtual size get_element_size() const = 0;
	virtual ~ui_element(){}

protected:
	point position_;
	window window_;
};

