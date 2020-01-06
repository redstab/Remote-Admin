#pragma once
#include "precompile.h"
#include "ui_element.h"

/// <summary>
/// Klass f�r att best�mma rikting f�r linjen
/// </summary>
enum class orientation {
	vertical = 0,
	horizontal = 1
};

/// <summary>
/// Klass f�r att skriva linjer
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

	// �rvda fr�n ui_element
	size get_element_size() const;
	void draw_element(); 

protected:
	int length_; // l�ngd p� linje
	orientation rotation_; // orientation av linje
};

