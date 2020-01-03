#pragma once
#include "window_log.h";

using func_map = std::unordered_map<std::string, std::function<void(std::string)>>; // alias för en map/dictionary med nyckeln string och datan en funktion som returnerar void men tar emot en sträng som parameter
//används för att skapa en konsol i en viss del av skärmen 
class command_line :
	public window_log // ärver från window_log som ärver från ui_element
{
public:
	command_line(window&, point, size, std::string, func_map); // skapa konsolen med ett föräldrar fönster, position, storlek, prompt-meddelande och en funktions map
	void prompt(); // för att acceptera ett kommando och exekevera om den finns i funk mappen
	bool alive(); // för att bryta loopen vid tex shutdown 
	void set_prompt(std::string prompt) { prompt_ = prompt; } // set funktion
	void set_functions(func_map fm) { functions_ = fm; } // set funktion
	std::string input_str(); // för att ta emot input: tex cin << s; -> input_str(2);
	void dö();
private:
	std::string prompt_; // prompt-meddelande
	func_map functions_; // map av funktioner
	bool alive_; // för att upperäthålla en loop
	std::string input(int y); // för att ta input vid en specifik y koordinat
	std::vector<std::string> argument_parser(std::string); // parsa sträng efter mellanslag för att seperera argument
	bool map_exist_execute(func_map, std::string, std::string);
	std::pair<std::string, std::string> split_input(std::string); // splita input mellan första ' '
};