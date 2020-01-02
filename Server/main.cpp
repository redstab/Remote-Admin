#include "precompile.h"
#include "tcp_server.h"
#include "curse.h"
#include "input_box.h"
#include "title.h"
#include "command_line.h"
#include "line.h"
#include "window_log.h"
#include "server.h"

int main()
{
	curse c;

	startup_wsa();

	const int padding = 5;

	window win({ COLS - padding * 2,LINES - padding });

	title titel(win, " Remote Administration Tool ", 1);
	line vertical_seperator(win, { win.get_size().x / 2, 3 }, win.get_size().y - 4, orientation::vertical);
	line horizontal_seperator(win, { 1, 2 }, win.get_size().x - 2, orientation::horizontal);

	server main(win, { 0,0 }, 54321, {&titel, &vertical_seperator, &horizontal_seperator});

	main.draw_element();

	win.show_border();

	main.bind();

	main.listen();

	main.startup();

	main.cli_loop();
}