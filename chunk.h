#ifndef IV_CHUNK_H
#define IV_CHUNK_H

#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

/* several whole contigous lines
 * 
 * we store the contents in a standard STL-like mutable container,
 * also store some persistent iterators in a map keyed by a mark_type
 *
 * we assume that all parent_type iterators are invalidated at any
 * non-const operation
 */
template <class Parent>
struct mutable_container_chunk : public Parent
{
	typedef Parent parent_type;
	using typename parent_type::value_type;
	using typename parent_type::const_iterator;
	using typename parent_type::iterator;
	typedef std::string mark_type;

	std::map<mark_type, iterator> marks;

protected:
	struct preserve_mark_positions {
		mutable_container_chunk *c;
		typename Parent::difference_type where;
		typename Parent::difference_type count;
		std::map<mark_type, typename Parent::difference_type> p;

		preserve_mark_positions(mutable_container_chunk *_c,
			typename Parent::difference_type _where,
			typename Parent::difference_type _count
		) : c(_c), where(_where), count(_count) {
			for (const auto e : c->marks)
				p.insert(std::make_pair(
					e.first,
					std::distance(c->begin(), e.second)
				));
		}
		~preserve_mark_positions() {
			for (const auto e : p)
				if (e.second >= where)
					c->marks[e.first] = c->begin() + e.second + count;
				else
					c->marks[e.first] = c->begin() + e.second;
		}
	};

public:
	using parent_type::size;
	int lines;

	mutable_container_chunk() : lines(0) { }

	template <class InputIterator>
	mutable_container_chunk(InputIterator b, InputIterator e)
		: parent_type(b, e)
		, lines(std::count(this->cbegin(), this->cend(), value_type('\n')))
	{
		if (!this->empty())
			assert(this->back() == '\n');
	}

	void push_back(value_type c)
	{
		{
			preserve_mark_positions _(this, 0, 0);
			parent_type::push_back(c);
		}
		lines += (c == '\n');
	}

	iterator insert(iterator before, value_type c)
	{
		iterator ret;
		{
			preserve_mark_positions _(this, before - this->begin(), 1);
			ret = parent_type::insert(before, c);
		}
		lines += (c == '\n');
		return ret;
	}

	iterator erase(iterator it)
	{
		iterator ret;
		char c = *it;
		{
			preserve_mark_positions _(this, it - this->begin(), -1);
			ret = parent_type::erase(it);
		}
		lines -= (c == '\n');
		return ret;
	}

	const_iterator mark(mark_type m) const
	{
		auto i = marks.find(m);
		if (i == marks.end())
			return Parent::end();
		return i->second;
	}

	iterator mark(mark_type m)
	{
		auto i = marks.find(m);
		if (i == marks.end())
			return Parent::end();
		return i->second;
	}

	void mark(mark_type m, iterator it)
	{
		if (it == Parent::end())
			marks.erase(m);
		else
			marks[m] = it;
	}
};

typedef mutable_container_chunk<std::vector<char>> chunk;

#endif // IV_CHUNK_H
