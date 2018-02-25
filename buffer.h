#include <algorithm>
#include <cassert>
#include <cstdlib> // std::rand
#include <iterator>
#include <list>
#include "chunk.h"

struct buffer : public std::list<chunk>
{
	struct const_iterator : public std::iterator<std::random_access_iterator_tag, char> {
		const buffer *b;
		std::list<chunk>::const_iterator list_it;
		chunk::const_iterator chunk_it;

		const_iterator() { }
		const_iterator(const buffer *_b, std::list<chunk>::const_iterator l, chunk::const_iterator c)
			: b(_b), list_it(l) , chunk_it(c)
		{
		}
		const_iterator &operator ++() {
			++chunk_it;
			if (chunk_it == list_it->end()) {
				++list_it;
				chunk_it = (list_it == b->std::list<chunk>::end()) ? __gnu_cxx::rope<char>::const_iterator() : list_it->begin();
			}
			return *this;
		}
		const_iterator &operator --() {
			if (chunk_it == list_it->begin())
				chunk_it = --(--list_it)->end();
			else
				--chunk_it;
			return *this;
		}

		const_iterator &operator +=(difference_type n)
		{
			auto x = std::min(n, list_it->end() - chunk_it);
			n -= x;
			chunk_it += x;
			while (n >= (++list_it)->chars)
				n -= list_it->chars;
			chunk_it = list_it->begin() + n;
			if (chunk_it == list_it->end())
				chunk_it = (++list_it)->begin();
			return *this;
		}

		difference_type operator -(const_iterator other) {
			if (list_it == other.list_it)
				return chunk_it - other.chunk_it;
			difference_type ret = list_it->end() - chunk_it;
			while (++list_it != other.list_it)
				ret += list_it->lines;
			return ret + (other.chunk_it - list_it->begin());
		}

		bool operator ==(const_iterator other) {
			return list_it == other.list_it && chunk_it == other.chunk_it;
		}
		bool operator !=(const_iterator other) {
			return !(*this == other);
		}

		char operator *() { return *chunk_it; }

		std::list<chunk>::const_iterator list_iter() const { return list_it; };
		chunk::const_iterator chunk_iter() const { return chunk_it; };
	};

	struct iterator : public const_iterator
	{
		iterator() { }
		iterator(buffer *_b, std::list<chunk>::iterator l, chunk::iterator c)
			: const_iterator(_b, l, c)
		{
		}

		std::list<chunk>::iterator list_iter() { return reinterpret_cast<std::list<chunk>::iterator &>(list_it); };
		chunk::iterator chunk_iter() { return reinterpret_cast<chunk::iterator &>(chunk_it); };
	};

	iterator start_, cursor_;
	std::string filename;

	buffer() : start_(begin()), cursor_(begin()) {
		assert(start_ == end());
	}
	buffer(const std::string _filename) : filename(_filename)
	{
		r();
	}

	const_iterator begin() const {
		auto b = std::list<chunk>::begin();
		auto e = std::list<chunk>::end();
		return const_iterator(this, b, b == e ? __gnu_cxx::rope<char>::const_iterator() : b->begin());
	}
	iterator begin() {
		std::list<chunk>::iterator b = std::list<chunk>::begin();
		std::list<chunk>::iterator e = std::list<chunk>::end();
		return iterator(this, b, b == e ? __gnu_cxx::rope<char>::iterator() : b->mutable_begin());
	}
	const_iterator end() const { return const_iterator(this, std::list<chunk>::end(), __gnu_cxx::rope<char>::const_iterator()); }
	iterator end() { return iterator(this, std::list<chunk>::end(), __gnu_cxx::rope<char>::iterator()); }
	const_iterator start() const { return start_; }
	iterator start() { return start_; }
	const_iterator cursor() const { return cursor_; }
	iterator cursor() { return cursor_; }
	int cursor_x() const
	{
		return std::distance(cursor().list_it->begin(), cursor().chunk_it);
	}

	template <class Iterator>
	void assign(Iterator begin, Iterator end)
	{
		clear();
		push_back(chunk());
		std::list<chunk>::iterator l0 = std::list<chunk>::begin();
		int l = 0;
		for (; begin != end; ++begin) {
			switch (*begin) {
			case '\t':
				for (int i = 0; i < 8; i++)
					l0->push_back(' ');
				break;
			case '\n':
				l0->push_back(*begin);
				if (++l % 10 == 0) {
					push_back(chunk());
					++l0;
				}
				break;
			default:
				l0->push_back(*begin);
				break;
			}
		}
		start_ = cursor_ = this->begin();
	}

	int line(const_iterator i) const
	{
		int ret = 0;
		for (auto j = std::list<chunk>::begin(); j != i.list_iter(); ++j)
			ret += j->lines;
		if (i.list_iter() != std::list<chunk>::end())
			for (auto j = i.list_iter()->begin(); j != i.list_iter()->end() && j != i.chunk_iter(); ++j)
				if (*j == '\n')
					ret++;
		return ret;
	}

	int lines() const {
		return line(end());
	}

	const_iterator line(int n) const
	{
		std::list<chunk>::const_iterator i = std::list<chunk>::begin();
		while (i != std::list<chunk>::end() && i->lines <= n)
			n -= i++->lines;
		if (i == std::list<chunk>::end())
			return end();
		chunk::const_iterator j = i->begin();
		while (n)
			n -= (*(j++) == '\n');
		return const_iterator(this, i, j);
	}

	iterator line(int n) {
		const_iterator ci = static_cast<const buffer *>(this)->line(n);
		return reinterpret_cast<iterator &>(ci);
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

	iterator insert(iterator i, char c)
	{
		if (i == end())
			i.chunk_it = (--i.list_it)->end();
		i.chunk_it = i.list_iter()->insert(i.chunk_iter(), c);
		return i;
	}

	void adjust_start()
	{
		while (cline() >= sline() + LINES - 2)
			++start_;
		while (cline() < sline())
			--start_;
		start_ = line(sline());
	}

	void set_start(int _start)
	{
		//std::cout << "!!!" << _start << std::endl;
		start_ = line(_start);
		while (cline() >= sline() + LINES - 2)
			--cursor_;
		while (cline() < sline())
			++cursor_;
	}

	void read(std::istream &stream)
	{
		assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
	}

	void write(std::ostream &stream)
	{
		for (auto c : *this)
			stream << c;
	}

	void r(std::string _filename = std::string())
	{
		if (_filename.empty())
			_filename = filename;
		std::ifstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		read(stream);
	}

	void o(const std::string &_filename = std::string())
	{
		if (_filename.empty())
			throw std::invalid_argument(":o needs an argument");
		std::ifstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		read(stream);
		filename = _filename;
	}

	void w(std::string _filename = std::string())
	{
		if (_filename.empty())
			_filename = filename;
		std::ofstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		write(stream);
	}

	void saveas(const std::string &_filename = std::string())
	{
		if (_filename.empty())
			throw std::invalid_argument(":saveas needs an argument");
		std::ofstream stream(_filename);
		stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		write(stream);
		filename = _filename;
	}
} buf;

