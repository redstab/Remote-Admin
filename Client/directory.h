#pragma once
#include "precompile.h"

using path = std::filesystem::path;
using dir_item = std::pair<path, int>; // path och storlek, storlek blir -9 när det är en mapp

//mapp explorer
class directory
{
public:

	directory() {};

	directory(std::string); // initializera klassen till en basmapp

	std::vector<dir_item> ascend(); // gå upp till föräldrar mappen och returnera den

	std::vector<dir_item> descend(int); // gå in i en undermapp med index och returnera mappen

	std::vector<dir_item> current_directory(); // returnera aktuella mappen

	std::string buffer(int);

private:

	path dir; // för att hålla koll på aktuell path

	std::vector<dir_item> current_directory_; // för att spara query

	std::vector<dir_item> index(); // för att indexera dir mappen

};

