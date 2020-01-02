#pragma once
#include <vector>
#include <filesystem>
#include <string>

using path = std::filesystem::path;
using dir_item = std::pair<path, int>;

//För att välja filer så måste vi ha en cli mapp explorer
class directory
{
public:

	directory(std::string);

	std::vector<dir_item> ascend();

	std::vector<dir_item> descend(int);

	std::vector<dir_item> current_directory();

	std::string buffer(int);

private:

	path dir;

	std::vector<dir_item> current_directory_;

	std::vector<dir_item> index();

};

