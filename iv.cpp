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
#include "text.h"

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

const int tab_size = 8;

struct buffer : public text<wchar_t>
{
	typedef text<wchar_t> parent_type;
	std::string filename;

	buffer() { marks[""] = marks["_"] = 0; }
	buffer(const std::string _filename) : filename(_filename)
	{
		r();
	}

	const_iterator start() const
	{
		return begin() + marks.find("")->second;
	}

	iterator start()
	{
		return begin() + marks.find("")->second;
	}

	const_iterator cursor() const
	{
		return begin() + marks.find("_")->second;
	}

	iterator cursor()
	{
		return begin() + marks.find("_")->second;
	}

	int cursor_x() const
	{
		return std::distance(line(cline()), cursor());
	}

	template <class Iterator>
	void assign(Iterator begin, Iterator end)
	{
		parent_type::assign(begin, end);
		marks[""] = marks["_"] = 0;
	}

	int cline() const
	{
		return line(cursor());
	}

	int sline() const
	{
		return line(start());
	}

	int cline_size() const
	{
		return std::distance(line(cline()), line(cline() + 1));
	}

	int file_lines() const
	{
		return LINES - 2;
	}

	void adjust_start()
	{
		if (sline() < cline() - file_lines() + 1)
			marks[""] = line(cline() - file_lines() + 1) - begin();
		if (cline() < sline())
			marks[""] = line(cline()) - begin();
		marks[""] = line(sline()) - begin();
	}

	void set_start(int _start)
	{
		//std::cout << "!!!" << _start << std::endl;
		marks[""] = line(_start) - begin();
		if (cline() >= sline() + file_lines())
			marks["_"] = line(sline() + file_lines() - 1) - begin();
		if (cline() < sline())
			marks["_"] = start() - begin();
	}

	void read(std::wistream &stream)
	{
		stream.imbue(std::locale("en_US.UTF-8"));
		assign(std::istreambuf_iterator<wchar_t>(stream), std::istreambuf_iterator<wchar_t>());
	}

	void write(std::wostream &stream)
	{
		stream.imbue(std::locale("en_US.UTF-8"));
		std::copy(begin(), end(), std::ostreambuf_iterator<wchar_t>(stream));
	}

	void r(std::string _filename = std::string())
	{
		if (_filename.empty())
			_filename = filename;
		std::wifstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		read(stream);
	}

	void o(const std::string &_filename = std::string())
	{
		if (_filename.empty())
			throw std::invalid_argument(":o needs an argument");
		std::wifstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		read(stream);
		filename = _filename;
	}

	void w(std::string _filename = std::string())
	{
		if (_filename.empty())
			_filename = filename;
		std::wofstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		write(stream);
	}

	void saveas(const std::string &_filename = std::string())
	{
		if (_filename.empty())
			throw std::invalid_argument(":saveas needs an argument");
		std::wofstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		write(stream);
		filename = _filename;
	}
} buf;

enum class mode_type {
	NORMAL,
	INSERT,
	COMMAND
} mode;

struct screen_initializer
{
	screen_initializer() {
		std::cerr << "Locale set" << std::endl;
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
	if (std::isprint(c))
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