#pragma once
#include "precompile.h"

//struktur för att abstraktera koordinater på skärmen
struct point
{
	int x = 0;
	int y = 0;
};

using size = point; // samma struktur kan också användas för att beskriva storlek