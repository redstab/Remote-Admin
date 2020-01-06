#pragma once
#include "precompile.h"
#include "ui_element.h"
// klass för att hantera wireframe eller de delar som inte ändras vid varje omskrivning tex linjer och titlar
// man skulle kunna tycka att denna klass ska vara barn till ui_element
// men jag tycker att den här klassen bara ska hantera ui_elementen som separata instanser
class wireframe 
{
public:
	wireframe(std::vector<ui_element*> elems) : elements(elems) {}
	std::vector<ui_element*>& get_elements() { return elements; } 
	void draw() { // rita varje element
		for (auto e : elements) {
			e->draw_element();
		}
	}
private:
	std::vector<ui_element*> elements;
};

