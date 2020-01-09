#include "precompile.h"
#include "directory.h"
directory::directory(std::string begin_path) : dir(begin_path) { index(); }

std::vector<dir_item> directory::ascend()
{
	dir = dir.parent_path(); // g� till f�r�ldrar mappen samma som om skriva descend(0) pga ..
	return index(); //returnera f�r�ldrar mappen
}

std::vector<dir_item> directory::descend(int index)
{
	if (index >= 0 && index < static_cast<int>(current_directory_.size())) { // bounds check
		auto item = current_directory_[index].first;
		if (item.extension() == "") {// is folder
			dir /= item; // append end -> (c:\\) / (books) = c:\books
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
	}); // max l�ngden p� filnamnet

	auto max_size_iter = std::max_element(current_directory_.begin(), current_directory_.end(), [&](dir_item a, dir_item b) {
		return a.second < b.second;
	}); // max l�ngd p� fil storleken

	// konvertera till v�rden
	int max_name = max_name_iter->first.filename().string().length(); 
	int max_size = std::to_string(max_size_iter->second).length();

	//skriv sprintf format
	std::string format = "%-" + std::to_string(max_length - max_size - 1) + "s";

	std::string container = dir.string() + "\n";

	for (auto [del, storlek] : current_directory_) {
		std::vector<char> buffer(max_length+2, '\0'); // allokera buffer till filnamn + padding + filstorlek 
		if (storlek == -9) { // mapp
			buffer.resize(sprintf(&buffer[0],(format + "\n").c_str(), ("/" + del.filename().string()).c_str())); // utf�r format och g�r om storleken p� buffern
		}
		else { // fil
			buffer.resize(sprintf(&buffer[0],(format + "%" + std::to_string(max_size) + "d\n").c_str(), del.filename().string().c_str(), storlek));// utf�r format och g�r om storleken p� buffern
		}
		container += std::string(buffer.begin(), buffer.end()); // l�gg till buffern till str�ngen 
	}

	//denna sprintf metod fungerar bara n�r man vet max storleken p� sin data 

	return container; // returnera alla buffrar

}


std::vector<dir_item> directory::index()
{
	std::vector<dir_item> directory_items{ {"..", -9} }; // f�r att kunna g� tillbaka
	std::error_code error;
	auto iterator = std::filesystem::directory_iterator(dir, error); // skapa iterator blir std::filesystem::directory_iterator() vid error
	if (iterator != std::filesystem::directory_iterator()) {
		for (auto p : iterator) { // g� igenom mappen

			if (std::filesystem::path(p).extension() == "") { // is folder
				directory_items.push_back({ std::filesystem::path(p), -9 }); // l�gg till mapp med storlek -9
			}
			else {
				std::error_code er;
				int file_size = static_cast<int>(std::filesystem::file_size(p, er)); // f� storlek
				directory_items.push_back({ std::filesystem::path(p), file_size}); // l�gg till path med storlek
			}
		}
	}
	else { // vid error s� �r vi i en mapp som vi inte har tillg�ng till allts� g� upp i niv� och indexa igen
		directory_items =  ascend(); // rekursive kallande kan fasna om vi har en cyklisk mapp struktur
	}

	current_directory_ = directory_items; // spara indexeringen f�r ascend

	return directory_items;
}
