#pragma once
#include "precompile.h"
// simpel wrapper klass f�r ncurses init
class curse
{
public:
	curse() {
		initscr();
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		nonl();
		refresh();
	}
	~curse() {
		endwin();
	}
};

