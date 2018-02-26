#include <algorithm>
#include <cassert>
#include <cctype> /* isprint */
#include <cstdlib> /* exit() */
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include "buffer.h"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

const int tab_size = 8;

enum class mode_type {
	NORMAL,
	INSERT,
	COMMAND
} mode;

struct screen_initializer
{
	screen_initializer() { initscr(); }
};

struct Window : public screen_initializer
{
	WINDOW *file;
	WINDOW *status;
	WINDOW *cmdline;
	std::string command;

	Window();
	~Window();
	int input() { return wgetch(file); }
	void update();
	void update_file();
	void update_status();
	void update_cmdline();
	void activate_window();
} win;

Window::Window()
	: screen_initializer(),
	file(newwin(LINES - 2, COLS, 0, 0)),
	status(newwin(1, COLS, LINES - 2, 0)),
	cmdline(newwin(1, COLS, LINES - 1, 0))
{
	clear();
	noecho();
	cbreak();
	for (WINDOW *w: {stdscr, file, status, cmdline})
		keypad(w, TRUE);
}

Window::~Window()
{
	endwin();
}

static void activate(WINDOW *w)
{
	wrefresh(w);
}

void Window::update()
{
	clear();
	update_file();
	update_status();
	update_cmdline();
	for (WINDOW *w: {file, status, cmdline})
		wnoutrefresh(w);
	activate_window();
	doupdate();
}

void Window::update_file()
{
	wclear(file);
	int firstline = buf.sline();
	wmove(file, 0, 0);
	//int x = 0;
	//for (std::list<chunk>::const_iterator i = buf.std::list<chunk>::begin(); i != buf.std::list<chunk>::end(); ++i)
	//	std::cout << ">" << i->size() << std::endl;
	if (buf.empty()) {
		assert(chunk::const_iterator() == (chunk::const_iterator)chunk::iterator());
		assert(buf.begin() == buf.end());
		assert(buf.start().list_iter() == buf.std::list<chunk>::end());
		assert(buf.start().chunk_iter() == chunk::iterator());
		assert(buf.start() == buf.end());
	}
	for (buffer::const_iterator i = buf.start(); i != buf.end(); ++i) {
		assert(i.list_iter() != buf.std::list<chunk>::end());
		assert(i.list_iter()->end() != i.chunk_iter());
		waddch(file, *i);
	}
	int cline = buf.cline();
	/*
	wmove(file, 5, 0);
	std::ostringstream o;
	//o << cline << *buf.cursor() << std::endl;
	o << buf.line(buf.start()) << std::endl;
	waddstr(file, o.str().c_str());
	*/
	wmove(file, cline - firstline, std::distance(buf.line(buf.cline()), buf.cursor()));
}

void Window::update_status()
{
	wclear(status);
	waddstr(status, buf.filename.empty() ? "Untitled" : buf.filename.c_str());
	if (mode == mode_type::INSERT)
		waddstr(status, " ---INSERT---");
	wrefresh(status);
}

void Window::update_cmdline()
{
	wclear(cmdline);
	if (mode == mode_type::COMMAND) {
		wprintw(cmdline, ":");
		wprintw(cmdline, command.c_str());
	}
	wrefresh(cmdline);
}

void Window::activate_window()
{
	switch (mode) {
	case mode_type::NORMAL:
		activate(file);
		break;
	case mode_type::INSERT:
		activate(file);
		break;
	case mode_type::COMMAND:
		activate(cmdline);
		break;
	}
}

#include "handle_command.cpp"

struct key_bindings
{
	typedef std::pair<const int, std::function<void ()>> binding;
	std::map<binding::first_type, binding::second_type> bindings;
	key_bindings(const std::initializer_list<binding> &_bindings) : bindings(_bindings) { }
	bool handle(int key);
	void add_command_binding(int key, const char *cmd);
};

bool key_bindings::handle(int key)
{
	bool ret = bindings.count(key);
	if (ret)
		bindings[key]();
	return ret;
}

void key_bindings::add_command_binding(int key, const char *cmd)
{
	bindings.emplace(key, std::bind(handle_command, cmd));
}

key_bindings any_bindings({});

key_bindings normal_bindings({});
key_bindings insert_bindings({});
key_bindings command_bindings({});

void handle_key()
{
	int c = win.input();
	do {
		if (any_bindings.handle(c))
			break;
		if (mode == mode_type::NORMAL && normal_bindings.handle(c))
			break;
		if (mode == mode_type::INSERT && insert_bindings.handle(c))
			break;
		if (mode == mode_type::COMMAND && command_bindings.handle(c))
			break;
		/*
		if (mode == mode_type::INSERT && std::isprint(c)) {
			buf.insert(buf.cursor_, c);
			++buf.cursor_;
			win.update_file();
			break;
		}
		*/
		if (mode == mode_type::COMMAND && std::isprint(c)) {
			char string[] = {(char)c, '\0'};
			win.command.push_back(c);
			wprintw(win.cmdline, string);
			wrefresh(win.cmdline);
			break;
		}
		flash();
	} while (false);
}

bool quit_on_sigint = false;

void sigint_handler(int)
{
	if (quit_on_sigint)
		std::exit(0);
	mode = mode_type::NORMAL;
	win.update();
}

int main(int argc, char **argv)
{
	using namespace std::placeholders;

	signal(SIGINT, sigint_handler);

	if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [file]" << std::endl;
		return 1;
	} else if (argc < 2){
		wprintw(win.file, "IV -- simple vi clone");
		win.update_status();
	} else if (argv[1] == std::string("--dumpkeys")) {
		quit_on_sigint = true;
		while (true) {
			int c = win.input();
			wprintw(stdscr, "%d %s\n", c, key_name(c));
			wrefresh(stdscr);
		}
		return 0;
	} else {
		buf.o(argv[1]);
		win.update();
	}

	auto map = std::bind(&key_bindings::add_command_binding, &any_bindings, _1, _2);
	auto nmap = std::bind(&key_bindings::add_command_binding, &normal_bindings, _1, _2);
	auto imap = std::bind(&key_bindings::add_command_binding, &insert_bindings, _1, _2);
	auto cmap = std::bind(&key_bindings::add_command_binding, &command_bindings, _1, _2);

#include "config.cpp"

	while (true) {
		try {
			handle_key();
		} catch (const std::exception &exc) {
			wclear(win.status);
			wprintw(win.status, exc.what());
			wrefresh(win.status);
		}
	}
}
