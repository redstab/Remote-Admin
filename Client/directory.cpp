#include "precompile.h"
#include "directory.h"
directory::directory(std::string begin_path) : dir(begin_path) { index(); }

std::vector<dir_item> directory::ascend()
{
	dir = dir.parent_path(); // gå till föräldrar mappen samma som om skriva descend(0) pga ..
	return index(); //returnera föräldrar mappen
}

std::vector<dir_item> directory::descend(int index)
{
	if (index >= 0 && index < current_directory_.size()) { // bounds check
		auto item = current_directory_[index].first;
		if (item.extension() == "") {// is folder
			dir /= item; // append end -> (c:\\) / books = c:\books
			dir = dir.lexically_normal(); // fixa relativa directories ex normalize(c:\books\..) -> c:\ 
		}
	}
	return this->index(); // indexa nya mappen och returnera den
}

std::vector<dir_item> directory::current_directory()
{
	return current_directory_;
}

std::string directory::buffer(int max_length)
{
	auto max_name_iter = std::max_element(current_directory_.begin(), current_directory_.end(), [&](dir_item a, dir_item b) {
		return a.first.filename().string().length() < b.first.filename().string().length();
	}); // max längden på filnamnet

	auto max_size_iter = std::max_element(current_directory_.begin(), current_directory_.end(), [&](dir_item a, dir_item b) {
		return a.second < b.second;
	}); // max längd på fil storleken

	// konvertera till värden
	int max_name = max_name_iter->first.filename().string().length(); 
	int max_size = std::to_string(max_size_iter->second).length();

	//skriv sprintf format
	std::string format = "%-" + std::to_string(max_length - max_size - 1) + "s";

	std::string container = dir.string() + "\n";

	for (auto [del, storlek] : current_directory_) {
		std::vector<char> buffer(max_length+2, '\0'); // allokera buffer till filnamn + padding + filstorlek 
		if (storlek == -9) { // mapp
			buffer.resize(sprintf(&buffer[0],(format + "\n").c_str(), ("/" + del.filename().string()).c_str())); // utför format och gör om storleken på buffern
		}
		else { // fil
			buffer.resize(sprintf(&buffer[0],(format + "%" + std::to_string(max_size) + "d\n").c_str(), del.filename().string().c_str(), storlek));// utför format och gör om storleken på buffern
		}
		container += std::string(buffer.begin(), buffer.end()); // lägg till buffern till strängen 
	}

	//denna sprintf metod fungerar bara när man vet max storleken på sin data 

	return container; // returnera alla buffrar

}


std::vector<dir_item> directory::index()
{
	std::vector<dir_item> directory_items{ {"..", -9} }; // för att kunna gå tillbaka
	std::error_code error;
	auto iterator = std::filesystem::directory_iterator(dir, error); // skapa iterator blir std::filesystem::directory_iterator() vid error
	if (iterator != std::filesystem::directory_iterator()) {
		for (auto p : iterator) { // gå igenom mappen

			if (std::filesystem::path(p).extension() == "") { // is folder
				directory_items.push_back({ std::filesystem::path(p), -9 });
			}
			else {
				std::error_code er;
				int file_size = std::filesystem::file_size(p, er); // få storlek
				directory_items.push_back({ std::filesystem::path(p), file_size}); // lägg till path med storlek
			}
		}
	}
	else { // vid error så är vi i en mapp som vi inte har tillgång till alltså gå upp i nivå och indexa igen
		directory_items =  ascend(); // rekursive kallande kan fasna om vi har en cyklisk mapp struktur
	}

	current_directory_ = directory_items; // spara indexeringen för ascend

	return directory_items;
}
