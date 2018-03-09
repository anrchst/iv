/* Copyright (C) Anton Novikov, 2018 
 * distributed by terms of GPLv3
 * mrk, I love u
 */
#include <algorithm>
#include <cctype> /* isprint */
#include <cstdlib> /* system(), exit() */
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

buffer buf;

enum class mode_type {
	NORMAL,
	INSERT,
	COMMAND
} mode;

struct screen_initializer
{
	screen_initializer() {
		setlocale(LC_CTYPE, "");
		initscr();
	}
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
}

void Window::update_file()
{
	wclear(file);
	wmove(file, 0, 0);
	for (buffer::const_iterator i = buf.line(buf.sline()); i != buf.begin() + buf.marks["_"]; ++i) {
		cchar_t c = {};
		c.chars[0] = *i;
		wadd_wch(file, &c);
		if (getcury(file) >= buf.file_lines() - 1 && getcurx(file) >= COLS - 1)
			break;
	}
	int cy = getcury(file);
	int cx = getcurx(file);
	for (buffer::const_iterator i = buf.begin() + buf.marks["_"]; i != buf.end(); ++i) {
		waddch(file, *i);
		if (getcury(file) >= buf.file_lines() - 1 && getcurx(file) >= COLS - 1)
			break;
	}
	wmove(file, cy, cx);
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
	typedef std::pair<const wchar_t, std::function<void ()>> binding;
	std::map<binding::first_type, binding::second_type> bindings;
	key_bindings(const std::initializer_list<binding> &_bindings) : bindings(_bindings) { }
	bool handle(wchar_t key);
	void add_command_binding(wchar_t key, const char *cmd);
};

bool key_bindings::handle(wchar_t key)
{
	bool ret = bindings.count(key);
	if (ret)
		bindings[key]();
	return ret;
}

void key_bindings::add_command_binding(wchar_t key, const char *cmd)
{
	bindings.emplace(key, std::bind(handle_command, cmd));
}

key_bindings any_bindings({});

key_bindings normal_bindings({});
key_bindings insert_bindings({});
key_bindings command_bindings({});

bool is_viewable(wchar_t c) {
	if (std::iswprint(c))
		return true;
	if (c == '\t' || c == '\n')
		return true;
	return false;
}

void handle_key()
{
	wchar_t c = win.input();
	do {
		if (any_bindings.handle(c))
			break;
		if (mode == mode_type::NORMAL && normal_bindings.handle(c))
			break;
		if (mode == mode_type::INSERT && insert_bindings.handle(c))
			break;
		if (mode == mode_type::COMMAND && command_bindings.handle(c))
			break;
		if (mode == mode_type::INSERT && is_viewable(c)) {
			buf.insert(c);
			win.update_file();
			break;
		}
		if (mode == mode_type::COMMAND && std::isprint(c)) {
			win.command.push_back(c);
			waddch(win.cmdline, c);
			wrefresh(win.cmdline);
			break;
		}
		flash();
	} while (false);
}

void sigint_handler(int)
{
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
		std::ostringstream hello;
		hello << "IV -- simple vi clone; sizeof(tree_node) = " << sizeof(buffer::tree_node_type);
		wprintw(win.file, hello.str().c_str());
		win.update_status();
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
