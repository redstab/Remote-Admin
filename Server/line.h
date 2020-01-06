#pragma once
#include "precompile.h"
#include "ui_element.h"

/// <summary>
/// Klass för att bestämma rikting för linjen
/// </summary>
enum class orientation {
	vertical = 0,
	horizontal = 1
};

/// <summary>
/// Klass för att skriva linjer
/// </summary>
class line :
	public ui_element
{
public:
	line(const window& win, point begin, int length, orientation rotation);

	// enkapsulerings metoder
	virtual void set_orientation(orientation rotation);

	orientation get_orientation() const;

	void set_length(int length);
	int get_length() const;

	// ärvda från ui_element
	size get_element_size() const;
	void draw_element(); 

protected:
	int length_; // längd på linje
	orientation rotation_; // orientation av linje
};

