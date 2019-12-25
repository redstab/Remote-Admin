#include "precompile.h"
#include "window.h"

window::window(size window_size, std::optional<point> position) : 
	window_size_{ window_size },
	screen_size_{ getmaxx(stdscr),getmaxy(stdscr) },
	position_{ (position.has_value()) ? *position : point({0,0}) },
	center_{ !position.has_value() },
	window_{ make_window(window_size_) }
{}


WINDOW* window::get_window() const
{
	return window_;
}

size window::get_size() const
{
	return window_size_;
}

void window::set_size(size window_size)
{
	window_size_ = window_size;
}

point window::get_position() const
{
	return position_;
}

void window::move_window(point position)
{
	clear();
	mvwin(window_, position.y, position.x);
}

void window::show_border()
{
	border_ = true;
	wborder(window_, 0, 0, 0, 0, 0, 0, 0, 0);
	wrefresh(window_);
}

void window::hide_border()
{
	border_ = false;
	wborder(window_, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(window_);
}

bool window::has_border() const
{
	return border_;
}


WINDOW* window::make_window(size window_size)
{
	return newwin(window_size.y, window_size.x, (center_) ? ((screen_size_.y - window_size.y) / 2) : position_.y, (center_) ? ((screen_size_.x - window_size.x) / 2) : position_.x);
}
