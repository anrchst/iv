#include <algorithm>
#include <cassert>
#include <cstdlib> // std::rand
#include <iterator>
#include <list>
#include <stdexcept>
#include "chunk.h"

struct buffer : public std::list<chunk>
{
	typedef std::list<chunk> C;

	struct const_iterator : public std::iterator<std::random_access_iterator_tag, char> {
		const buffer *b;
		C::const_iterator list_it;
		chunk::const_iterator chunk_it;

		bool at_end() const { return list_it == b->C::end(); }

		const_iterator() { }
		const_iterator(const buffer *_b, C::const_iterator l, chunk::const_iterator c)
			: b(_b), list_it(l) , chunk_it(c)
		{
		}
		const_iterator &operator ++() {
			assert(chunk_it != list_it->end());
			assert(!at_end());
			assert(chunk_it != chunk::const_iterator());
			++chunk_it;
			if (chunk_it == list_it->end()) {
				++list_it;
				chunk_it = at_end() ? chunk::const_iterator() : list_it->begin();
			}
			return *this;
		}
		const_iterator &operator --() {
			// also skip empty chunks
			int i = 0;
			while (list_it != b->c_begin() && chunk_it == list_it->begin())
				chunk_it = (--list_it)->end(), ++i;
			if (list_it == b->c_begin() && chunk_it == list_it->begin())
				throw std::runtime_error("decrementing begin() iterator");
			assert(chunk_it > list_it->begin());
			--chunk_it;
			return *this;
		}

		const_iterator &operator +=(difference_type n)
		{
			auto x = std::min(n, list_it->end() - chunk_it);
			n -= x;
			chunk_it += x;
			while (n >= (difference_type)(++list_it)->size())
				n -= list_it->size();
			chunk_it = list_it->begin() + n;
			if (chunk_it == list_it->end())
				chunk_it = (++list_it)->begin();
			return *this;
		}

		difference_type operator -(const_iterator other) {
			if (list_it == other.list_it)
				return chunk_it - other.chunk_it;
			difference_type ret = list_it->end() - chunk_it;
			for (auto i = list_it; i != other.list_it; ++i)
				ret += i->size();
			return ret + (other.chunk_it - other.list_it->begin());
		}

		bool operator ==(const_iterator other) {
			return list_it == other.list_it && chunk_it == other.chunk_it;
		}
		bool operator !=(const_iterator other) {
			return !(*this == other);
		}

		char operator *() { return *chunk_it; }

		C::const_iterator list_iter() const { return list_it; };
		chunk::const_iterator chunk_iter() const { return chunk_it; };
	};

	struct iterator : public const_iterator
	{
		iterator() { }
	
	private:
		iterator(const const_iterator &other)
			: const_iterator(other)
		{
		}

	public:
		C::iterator list_iter() const {
			return reinterpret_cast<const C::iterator &>(list_it);
		}
		chunk::iterator chunk_iter() const
		{
			if (at_end())
				return chunk::iterator();
			else {
				auto ret = list_iter()->begin();
				std::advance(ret, std::distance(list_it->cbegin(), chunk_it));
				return ret;
			}
		};
		iterator &operator ++() {
			const_iterator::operator ++();
			return *this;
		}
		iterator &operator --() {
			const_iterator::operator --();
			return *this;
		}
	};

	std::string filename;

	buffer() {
	}
	buffer(const std::string _filename) : filename(_filename)
	{
		r();
	}

	typename C::const_iterator c_begin() const { return C::begin(); }
	typename C::const_iterator c_end() const { return C::end(); }
	typename C::iterator c_begin() { return C::begin(); }
	typename C::iterator c_end() { return C::end(); }

	const_iterator begin() const {
		auto b = c_begin();
		auto e = c_end();
		return const_iterator(this, b, b == e ? chunk::const_iterator() : b->begin());
	}
	const_iterator end() const {
		return const_iterator(this, C::end(), chunk::const_iterator());
	}

	iterator begin()
	{
		const buffer *t = this;
		const_iterator ret = t->begin();
		return reinterpret_cast<const iterator &>(ret);
	}

	iterator end()
	{
		const buffer *t = this;
		const_iterator ret = t->end();
		return reinterpret_cast<const iterator &>(ret);
	}

	const_iterator mark(chunk::mark_type m) const
	{
		for (auto i = c_begin(); i != c_end(); ++i) {
			auto mark = i->mark(m);
			if (mark != i->end())
				return const_iterator(this, i, mark);
		}
		return end();
	}

	iterator mark(chunk::mark_type m)
	{
		const buffer *t = this;
		const_iterator ret = t->mark(m);
		return reinterpret_cast<const iterator &>(ret);
	}

	void mark(chunk::mark_type m, iterator it)
	{
		for (auto i = c_begin(); i != c_end(); ++i)
			i->mark(m, i->end());
		if (m == "" || m == "_")
			assert(it != end() || C::empty());
		if (it != end())
			it.list_iter()->mark(m, it.chunk_iter());
	}

	const_iterator start() const { return mark(""); }
	const_iterator cursor() const { return mark("_"); }
	iterator start() { return mark(""); }
	iterator cursor() { return mark("_"); }

	int x(const_iterator i) const
	{
		return std::distance(i.list_it->begin(), i.chunk_it);
	}

	int cursor_x() const
	{
		return x(cursor());
	}

	template <class Iterator>
	void assign(Iterator b, Iterator e)
	{
		clear();
		push_back(chunk());
		C::iterator l0 = C::begin();
		int l = 0;
		for (; b != e; ++b) {
			switch (*b) {
			case '\t':
				for (int i = 0; i < 8; i++)
					l0->push_back(' ');
				break;
			case '\n':
				l0->push_back(*b);
				if (++l % 10 == 0) {
					push_back(chunk());
					++l0;
				}
				break;
			default:
				l0->push_back(*b);
				break;
			}
		}
		mark("", begin());
		mark("_", begin());
	}

	int line(const_iterator i) const
	{
		int ret = 0;
		for (auto j = C::begin(); j != i.list_iter(); ++j)
			ret += j->lines;
		if (i.list_iter() != C::end())
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
		C::const_iterator i = C::begin();
		while (i != C::end() && i->lines <= n)
			n -= i++->lines;
		if (i == C::end())
			return end();
		chunk::const_iterator j = i->begin();
		while (n)
			n -= (*(j++) == '\n');
		return const_iterator(this, i, j);
	}

	iterator line(int n)
	{
		const buffer *t = this;
		const_iterator ret = t->line(n);
		return reinterpret_cast<const iterator &>(ret);
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
		iterator s = start();
		while (cline() >= line(s) + LINES - 2)
			++s;
		while (cline() < line(s))
			--s;
		mark("", line(line(s)));
	}

	void set_start(int _start)
	{
		if (_start >= lines())
			_start = lines() - 1;
		if (_start < 0)
			_start = 0;

		mark("", line(_start));
		iterator c = cursor();
		while (line(c) >= sline() + LINES - 2)
			--c;
		while (line(c) < sline())
			++c;

		mark("_", c);
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

