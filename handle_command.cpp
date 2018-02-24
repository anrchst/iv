void handle_command(const std::string &command)
{
	std::istringstream args(command);
	std::string arg0, arg1;
	args >> arg0;
	if (arg0 == "q" || arg0 == "quit") {
		exit(0);
	} else if (arg0 == "r") {
		std::string filename;
		if (args >> filename)
			buf.r(filename);
		else
			buf.r();
		win.update_file();
	} else if (arg0 == "w") {
		std::string filename;
		if (args >> filename)
			buf.w(filename);
		else
			buf.w();
	} else if (arg0 == "wq") {
		buf.w();
		exit(0);
	} else if (arg0 == "o" || arg0 == "e") {
		std::string filename;
		if (args >> filename)
			buf.o(filename);
		else
			buf.o();
		win.update_file();
		win.update_status();
	} else if (arg0 == "saveas") {
		std::string filename;
		if (args >> filename)
			buf.saveas(filename);
		else
			buf.saveas();
		win.update_status();
	} else if (arg0 == "cursor") {
		std::string direction;
		if (!(args >> direction))
			throw std::invalid_argument(":cursor needs an argument");
		if (direction == "left" && buf.cursor_x() > 0)
			buf.cursor_--;
		else if (direction == "right" && buf.cursor_x() < (int)buf.cline_size() - 1 - (mode == mode_type::NORMAL))
			buf.cursor_++;
		else if (direction == "up" && buf.cline() > 0) {
			auto cl = buf.cline();
			auto cx = std::distance(buf.line(cl), buf.cursor_);
			for (buf.cursor_ = buf.line(cl - 1); *buf.cursor() != '\n' && buf.cursor_x() < cx; buf.cursor_++)
				;
			buf.adjust_start();
		} else if (direction == "down") {
			auto cl = buf.cline();
			auto cx = std::distance(buf.line(cl), buf.cursor_);
			std::cout << cl << " " << cx << std::endl;
			if (cl < buf.lines() - 1)
				for (buf.cursor_ = buf.line(cl + 1); *buf.cursor() != '\n' && buf.cursor_x() < cx; buf.cursor_++)
					;
			buf.adjust_start();
		}
		win.update_file();
	} else if (arg0 == "0") {
		buf.cursor_ = buf.begin();
		win.update_file();
	} else if (arg0 == "100") {
		buf.cursor_ = buf.line(100);
		win.update_file();
	} else if (arg0 == "refresh") {
		win.update();
	} else if (arg0 == "mode") {
		std::string _mode;
		if (args >> _mode) {
			mode = _mode == "normal" ? mode_type::NORMAL :
			       _mode == "insert" ? mode_type::INSERT :
			                           mode_type::COMMAND;
			if (mode == mode_type::COMMAND)
				win.command = std::string();
			win.update();
		}
	} else if (arg0 == "page") {
		std::string direction;
		if (args >> direction) {
			if (direction == "up") {
				buf.set_start(std::max(0, buf.line(buf.start()) - LINES + 2));
			} else if (direction == "down") {
				buf.set_start(buf.line(buf.start()) + LINES - 2);
			}
			win.update_file();
		}
	} else if (arg0 == "halfpage") {
		std::string direction;
		if (args >> direction) {
			if (direction == "up") {
				buf.set_start(std::max(0, (int)buf.sline() - LINES / 2 + 1));
			} else if (direction == "down") {
				buf.set_start((int)buf.sline() + LINES / 2 - 1);
			}
			win.update_file();
		}
	} else if (arg0 == "n_0") {
		buf.cursor_ = buf.line(buf.cline());
		win.update_file();
	} else if (arg0 == "n_$") {
		while (*buf.cursor() != '\n')
			buf.cursor_++;
		buf.cursor_--;
		win.update_file();
	} else if (arg0 == "n_i") {
		mode = mode_type::INSERT;
		win.update();
	} else if (arg0 == "n_a") {
		buf.cursor_++;
		mode = mode_type::INSERT;
		win.update();
	} else if (arg0 == "n_I") {
		while(buf.cursor_ != buf.end() && *buf.cursor() != '\n')
			buf.cursor_--;
		buf.cursor_++;
		mode = mode_type::INSERT;
		win.update();
	} else if (arg0 == "n_A") {
		while(++buffer::const_iterator(buf.cursor_) != buf.end() && *buf.cursor() != '\n')
			buf.cursor_++;
		mode = mode_type::INSERT;
		win.update();
	} else if (arg0 != "misc") {
		throw std::invalid_argument("unknown command: " + arg0);
	} else if (!(args >> arg1)) {
		throw std::invalid_argument("need argument: " + arg0);
		/*
	} else if (arg1 == "i:backspace") {
		if (buf.cursor_x > 0) {
			buf.cursor->erase(buf.cursor_x--, 1);
			win.update_file();
		}
	} else if (arg1 == "c:backspace") {
		if (win.command.empty())
			mode = mode_type::NORMAL;
		else
			win.command.pop_back();
		win.update_cmdline();
		win.activate_window();
	} else if (arg1 == "escape") {
		if (mode == mode_type::INSERT)
			buf.cursor_x = std::min(std::max((int)buf.cursor->size() - 2, 0), std::max(buf.cursor_x, 1) - 1);
		mode = mode_type::NORMAL;
		win.update();
	*/
	} else if (arg1 == "c:return") {
		mode = mode_type::NORMAL;
		wclear(win.cmdline);
		wrefresh(win.cmdline);
		handle_command(win.command);
	}
}
