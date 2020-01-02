#pragma once
#include "precompile.h"
#include "point.h"
#include "window.h"

// abstrakt bas klass till alla element som visas på skärmen
class ui_element
{
public:
	ui_element(const window& win, point position);

	//enkapsulerings metoder
	point get_position() const; 
	window get_window() const;

	void set_position(point position);
	void set_window(const window& win);

	//för att flytta, skriva om och ta bort använder sig av virituella funcktionerna draw_element och get_element_size
	void move_element(point position);
	void clear_element() const;
	void redraw_element();

	// "pure"/abstrakt virtual funktion vilket innebär att barnen till klassen måste alltid definera draw_element om den inte hade varit "pure" så hade barnen inte behövt definera funktionen eftersom att definetionen redan finns i basklassen
	virtual void draw_element() = 0; 
	virtual size get_element_size() const = 0;
	virtual ~ui_element(){} // måste defineras så att barnens destructor kallas

protected:
	point position_;
	window window_;
};

