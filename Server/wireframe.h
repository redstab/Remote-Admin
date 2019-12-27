#pragma once
#include "precompile.h"
#include "ui_element.h"
class wireframe // klass för att hantera wireframe eller de delar som inte ändras vid varje omskrivning tex linjer och titlar
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

