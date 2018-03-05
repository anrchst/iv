#ifndef IV_TEXT_H
#define IV_TEXT_H

#include <deque>
#include <map>
#include <string>
#include "list.h"

template <class T>
struct returns_tag
{
	typedef int value_type;
	static value_type leaf_value() { return 0; }
	template<class Node>
	static value_type singleton_value(Node *n) { return n->v == T('\n'); }
};

template <class T>
struct text : public iv::list<T, returns_tag<T>>
{
	typedef iv::list<T, returns_tag<T>> parent_type;
	using typename parent_type::const_iterator;
	using typename parent_type::iterator;
	using parent_type::begin;
	using parent_type::end;
	using parent_type::root;
	std::map<std::string, int> marks;

	text() { marks[""] = 0; }

	void insert(char c)
	{
		int cursor = marks["_"];
		parent_type::insert(begin() + cursor, c);
		for (auto &m : marks) {
			if (m.second >= cursor)
				m.second++;
		}
	}

	void erase()
	{
		int cursor = marks["_"];
		parent_type::erase(begin() + cursor);
		for (auto m : marks) {
			if (m.second > cursor)
				m.second--;
		}
	}

	int lines() const
	{
		return root().stat();
	}

	int line(const_iterator i) const
	{
		int ret = 0;
		for (const_iterator j = i, k = i; j != end(); k = j, j = j.parent()) {
			//std::cout << *i << '|' << *j << "||" << *k << std::endl;
			if (k != j.left()) {
				if (j != i) {
					ret += (*j == T('\n'));
					//std::cout << "1. ret += " << (*j == T('\n')) << std::endl;
				}
				ret += j.left().stat();
				//std::cout << "2. ret += " << j.left().stat() << std::endl;
			}
			
		}
		return ret;
	}

	const_iterator line(int index) const
	{
		if (index < 0)
			return end();
		else if (index == 0)
			return begin();
		if (root().stat() <= index)
			return line(lines() - 1);
		const_iterator ret = root();
		while (ret.left().stat() != index) {
			if (ret.left().stat() < index) {
				index -= ret.left().stat() + (*ret == T('\n'));
				ret = ret.right();
			} else
				ret = ret.left();
		}
		while (*--ret != T('\n'))
			;
		return ++ret;
	}
	iterator line(int index)
	{
		return this->make_iterator(const_cast<const text *>(this)->line(index));
	}
	template <class Iterator>
	void assign(Iterator begin, Iterator end)
	{
		std::deque<char> d(begin, end);
		iv::list<T, returns_tag<T>>::operator =(iv::list<T, returns_tag<T>>(d.begin(), d.end()));
	}

	std::string str() const
	{
		std::string ret;
		for (const_iterator i = begin(); i != end(); ++i)
			ret.push_back(*i);
		return ret;
	}
};

#endif // IV_TEXT_H
