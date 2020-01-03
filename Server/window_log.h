#pragma once
#include "ui_element.h"

//klass som används som en log eller en read only konsol
class window_log :
	public ui_element
{
public:
	window_log(window& win, point position, size max_size, bool); // konstruktor
	window_log& operator<<(std::string); // vid klass << sträng
	window_log& operator++(); // vid klass++;
	window_log& operator--(); // vid klass--;
	void append(std::string); // för att printa text till konsolen
	void scroll(int); // för att skrolla konsolen
	void clear();
	//virituella funktioner som ärvs av ui_element
	size get_element_size() const; 
	void draw_element();

	WINDOW* get_derived();

protected:
	WINDOW* derived_; // barn fönstret till window_.get_window() i ui_element
	size max_size_; // max storlek för barn fönstret
	int max_char_count_; // max antal karaktärer i fönstret
	int max_lenght_; // max längd för en rad
	int char_count_; // antal karaktärer i buffern
	int cursor_; // från vilken rad utskrift sker ifrån
	bool auto_scroll_; // om man ska autoskrolla när man har skrivit en hel max_char_count_
	std::string data_; // buffer
};

