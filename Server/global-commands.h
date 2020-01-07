#pragma once
#include "precompile.h"
#include "server.h"

func_map server::get_global_commands()
{
	return {

		{"hide", [&](std::string args) {
			argument_handler({

				{"log", [&](std::string args) {
					// g�m logger och konsole
					console_log.clear_element();
					console.clear_element();
					// s�tt logger status till hidden
					console_log.hide();
					// ny storlek s� att konsolen tar hela sk�rmen
					console.resize({ window_.get_size().x - 2, console.get_element_size().y });
					console.clear();
					console.redraw_element(); // skriv ut konsolen igen
				}}

			}, args, "hide");
		}},
		{"unhide", [&](std::string args) {
			argument_handler({

				{"log", [&](std::string args) {
					console_log.unhide(); // s�tt status till shown
					console.resize({ window_.get_size().x / 2 - 1, window_.get_size().y - 4 }); // byt till gammla storleken 
					console.redraw_element(); // skriv ut konsolen igen
					console_log.redraw_element(); // skriv ut loggern igen
					wire.draw(); // skriv ut wireframen igen
				}}

			}, args, "unhide");
		}},

		{ "shutdown", [&](std::string args) { // st�ng ner servern 
			argument_parser(
				[&](std::string value) {
					alive = false; // d�da packet och client hanteraren
					console.d�(); // d�da konsolen (bara en fancy animation)
			}, args, "shutdown");
		}},

		{ "scroll", [&](std::string args) { // skrolla ett f�nster

			argument_handler({

				{"log", [&](std::string value) { // skrolla log
					if (value == "reset") { // om man ska reseta scrollen till botten
						while (console_log.scroll(1)) {}
						console_log.draw_element();
					}
					else {
						console << "scrolling " << args;
						scroll(console_log);
						console << " -> done\n";
					}
				}},

				{"command", [&](std::string value) { // skrolla konsolen
					if (value == "reset") { // om man ska reseta scrollen till botten
						while (console.scroll(1)) {}
						console.draw_element();
					}
					else {
						console << "scrolling " << args;
						scroll(console);
						console << " -> done\n";
					}
				}}

			}, args, "scroll");

		}},

		{ "help", [&](std::string args) { // visa hj�lp
			console << "\n";
			for (auto [func, doc] : (attached != nullptr ? klient_help : server_help)) {
				console << func << " - " << doc << "\n";
			}
			console << "\n";
		}},

		{ "clear",[&](std::string args) { // rensa ett f�nster
			argument_parser([&](std::string value) {
				if (args == "log") { // rensa loggen 
					console_log.clear();
					console_log.draw_element();
				}
				else if (args == "this" || args == "") { // rensa konsolen
					console.clear();
					console.draw_element();
				}
				else { // args inte en konsol eller f�nster
					console << "Cannot clear a window that does not exsists\n";
				}
			}, args, "clear");
		}},

		{"log",[&](std::string args) {
			argument_parser([&](std::string value) {
				if (!args.empty()) {
					std::unordered_map<std::string, int> log_values = {
						{"info", LOG_INFO},
						{"verbose", LOG_VERBOSE},
						{"sverbose", LOG_SUPER_VERBOSE},
						{"everything", LOG_ALL}
					};

					auto arguments = argument_slicer(args);

					int log_value = 0;

					for (auto option : arguments) {
						if (log_values.count(option)) {
							log_value |= log_values[option];
						}
					}

					console_log.set_log_level(log_value);
				}
				else {
					console << "The syntax of the command is incorrect. try log -h\n";
				}

			}, args, "log");
		}}
	};
}