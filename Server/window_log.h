#pragma once
#include "ui_element.h"

//klass som anv�nds som en log eller en read only konsol
class window_log :
	public ui_element
{
public:
	window_log(window& win, point position, size max_size, bool); // konstruktor
	window_log& operator<<(std::string); // vid klass << str�ng
	window_log& operator++(); // vid klass++;
	window_log& operator--(); // vid klass--;
	void append(std::string); // f�r att printa text till konsolen
	void scroll(int); // f�r att skrolla konsolen

	//virituella funktioner som �rvs av ui_element
	size get_element_size() const; 
	void draw_element();

protected:
	WINDOW* derived_; // barn f�nstret till window_.get_window() i ui_element
	size max_size_; // max storlek f�r barn f�nstret
	int max_char_count_; // max antal karakt�rer i f�nstret
	int max_lenght_; // max l�ngd f�r en rad
	int char_count_; // antal karakt�rer i buffern
	int cursor_; // fr�n vilken rad utskrift sker ifr�n
	bool auto_scroll_; // om man ska autoskrolla n�r man har skrivit en hel max_char_count_
	std::string data_; // buffer
};

