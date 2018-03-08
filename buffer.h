#ifndef BUFFER_H
#define BUFFER_H

#include "text.h"

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
};

#endif // BUFFER_H
