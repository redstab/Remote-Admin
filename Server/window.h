#pragma once
#include "precompile.h"
#include "point.h"

// klass f�r att abstraktera WINDOW klassen fr�n ncurses
// detta gjorde jag f�r att underl�tta och �ka s�kerheten p� programmet genom att wrappa up 
// en os�ker pekare i en klass. Underl�ttar genom att man kan l�tt flytta, g�mma f�nstret
// anv�nds av ui_element f�r att ha n�got att visa elementen p�
class window
{
public:
	
	window(size window_size = {120,30}, std::optional<point> begin_position = std::nullopt);

	window(const window& win) {
		window_size_ = win.window_size_;
		screen_size_ = win.screen_size_;
		position_ = win.position_;
		center_ = win.center_;
		window_ = win.window_;
	}

	WINDOW* get_window() const;

	size get_size() const;

	void set_size(size window_size);

	point get_position() const;

	void move_window(point position);

	void show_border();
	
	void hide_border();

	bool has_border() const;

	~window() {};

private:
	size window_size_{};
	size screen_size_{};
	point position_{};
	bool center_{};
	bool border_{};

	WINDOW* window_;
	
	WINDOW* make_window(size window_size);
};

