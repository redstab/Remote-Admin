#pragma once
#include "precompile.h"

//struktur f�r att abstraktera koordinater p� sk�rmen
struct point
{
	int x = 0;
	int y = 0;
};

using size = point; // samma struktur kan ocks� anv�ndas f�r att beskriva storlek