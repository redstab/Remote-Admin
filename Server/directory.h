#pragma once
#include "precompile.h"

using path = std::filesystem::path;
using dir_item = std::pair<path, int>; // path och storlek, storlek blir -9 n�r det �r en mapp

//mapp explorer
class directory
{
public:

	directory(std::string); // initializera klassen till en basmapp

	std::vector<dir_item> ascend(); // g� upp till f�r�ldrar mappen och returnera den

	std::vector<dir_item> descend(int); // g� in i en undermapp med index och returnera mappen

	std::vector<dir_item> current_directory(); // returnera aktuella mappen

	std::string buffer(int);

private:

	path dir; // f�r att h�lla koll p� aktuell path

	std::vector<dir_item> current_directory_; // f�r att spara query

	std::vector<dir_item> index(); // f�r att indexera dir mappen

};

