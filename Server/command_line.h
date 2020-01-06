#pragma once
#include "window_log.h";

using func_map = std::unordered_map<std::string, std::function<void(std::string)>>; // alias f�r en map/dictionary med nyckeln string och datan en funktion som returnerar void men tar emot en str�ng som parameter
//anv�nds f�r att skapa en konsol i en viss del av sk�rmen 
//�rver fr�n window_log f�r att kunna skapa en log p� en viss position av sk�rmen och f�r att en konsol �r en typ av window_log
class command_line :
	public window_log
{
public:
	command_line(window&, point, size, std::string, func_map); // skapa konsolen med ett f�r�ldrar f�nster, position, storlek, prompt-meddelande och en funktions map
	void prompt(); // f�r att acceptera ett kommando och exekevera om den finns i funk mappen
	bool alive(); // f�r att bryta loopen vid tex shutdown 
	void set_prompt(std::string prompt) { prompt_ = prompt; } // set funktion
	void set_functions(func_map fm) { functions_ = fm; } // set funktion
	std::string input_str(); // f�r att ta emot input: tex cin << s; -> input_str(2);
	void d�();
private:
	std::string prompt_; // prompt-meddelande
	func_map functions_; // map av funktioner
	bool alive_; // f�r att upper�th�lla en loop
	std::string input(int y); // f�r att ta input vid en specifik y koordinat
	std::vector<std::string> argument_parser(std::string); // parsa str�ng efter mellanslag f�r att seperera argument
	bool map_exist_execute(func_map, std::string, std::string);
	std::pair<std::string, std::string> split_input(std::string); // splita input mellan f�rsta ' '
};