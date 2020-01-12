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
	curse c; // starta ncurses
	std::locale::global(std::locale("sw")); // så att vi kan använda åäö
	//while (true) {
	//	char jkey = getch();
	//	printw("%d = %c\n", jkey, char(jkey));
	//	refresh();
	//}
	std::string cs = "åäöÅÄÖ";

	startup_wsa(); // starta winsocket 2.2

	const int padding = 5; // padding för fönstret

	window win({ COLS - padding * 2,LINES - padding }); // skapa main fönstret

	title titel(win, " Remote Administration Tool ", 1); // skapa titeln
	line vertical_seperator(win, { win.get_size().x / 2, 3 }, win.get_size().y - 4, orientation::vertical); // vertikala linjen som separerar konsolerna
	line horizontal_seperator(win, { 1, 2 }, win.get_size().x - 2, orientation::horizontal); // horizontella linjen som separerar titel från annat innehåll

	//skapa servern 
	// ui skrivs på win
	// startposition är {0,0}
	// lyssnar på port 54321 
	// wireframe är titel, vertical_seperator och horizontal_seperator
	server main(win, { 0,0 }, 54321, {&titel, &vertical_seperator, &horizontal_seperator}); 

	main.draw_element(); // skriv ut wireframe och konsolerna 

	win.show_border(); // visa border på fönstret

	main.bind(); // förbind porten till servern

	main.listen(); // säg till server socketen att börja lyssna på nya anslutningar

	main.startup(); // starta packet och klient hanterare i en ny tråd

	main.cli_loop(); // starta konsol loop för input från användaren
}