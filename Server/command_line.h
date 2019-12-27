#pragma once
#include "window_log.h";

using func_map = std::unordered_map<std::string, std::function<void(std::string)>>; // alias f�r en map/dictionary med nyckeln string och datan en funktion som returnerar void men tar emot en str�ng som parameter
//using func_map_int = std::unordered_map<std::string, std::function<void(const command_line&, std::string)>>; // alias f�r en map/dictionary med nyckeln string och datan en funktion som returnerar void men tar emot en str�ng som parameter

//anv�nds f�r att skapa en konsol i en viss del av sk�rmen 
class command_line :
	public window_log // �rver fr�n window_log som �rver fr�n ui_element
{
public:
	command_line(window&, point, size, std::string, func_map); // skapa konsolen med ett f�r�ldrar f�nster, position, storlek, prompt-meddelande och en funktions map
	void prompt(); // f�r att acceptera ett kommando och exekevera om den finns i funk mappen
	bool alive(); // f�r att bryta loopen vid tex shutdown 
	std::string input_str(int); // f�r att ta emot input: tex cin << s; -> input_str(2);
	void d�();
private:
	std::string prompt_; // prompt-meddelande
	func_map external_functions_; // map av ytre funktioner
	func_map internal_functions_ = { // map av internal funktioner f�r hantera konsolen
		{"clear", [&](std::string param) {data_.clear(); cursor_ = 0; }}
	};
	bool alive_; // f�r att upper�th�lla en loop
	std::string input(int y); // f�r att ta input vid en specifik y koordinat
	std::vector<std::string> argument_parser(std::string); // parsa str�ng efter mellanslag f�r att seperera argument
	bool map_exist_execute(func_map, std::string, std::string);
	std::pair<std::string, std::string> split_input(std::string); // splita input mellan f�rsta ' '
};

