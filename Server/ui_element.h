#pragma once
#include "precompile.h"
#include "point.h"
#include "window.h"

// bas klass till alla element som ska visas p� sk�rmen
// kan inte skapa instans av ui_element eftersom att vi inte vet vad ui_element �r f�r n�got
// d�rf�r skulle man kunna kalla ui_element som en abstrakt klass eller interface till ett ui_element p� sk�rmen
// eftersom att ui_element skulle kunna vara vad som helst s� �r draw_element och get_element_size abstrakta.
class ui_element
{
public:
	ui_element(const window& win, point position);

	//enkapsulerings metoder
	point get_position() const; 
	window get_window() const;

	void set_position(point position);
	void set_window(const window& win);

	//f�r att flytta, skriva om och ta bort anv�nder sig av virituella funcktionerna draw_element och get_element_size
	void move_element(point position);
	void clear_element() const;
	void redraw_element();

	// "pure"/abstrakt virtual funktion vilket inneb�r att barnen till klassen m�ste alltid definera draw_element om den inte hade varit "pure" s� hade barnen inte beh�vt definera funktionen eftersom att definetionen redan finns i basklassen
	virtual void draw_element() = 0; 
	virtual size get_element_size() const = 0;
	virtual ~ui_element(){} // m�ste defineras s� att barnens destructor kallas

protected:
	point position_;
	window window_;
};

