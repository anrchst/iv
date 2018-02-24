#ifndef IV_TEXT_H
#define IV_TEXT_H

#include <valarray>
#include "list.h"

struct subtree_size_tag
{
	typedef int value_type;
	static value_type leaf_value() { return 0; }
	template<class Node>
	static value_type singleton_value(Node *) { return 1; }
};

template <class T>
struct returns_tag
{
	typedef int value_type;
	static value_type leaf_value() { return 0; }
	template<class Node>
	static value_type singleton_value(Node *n) { return n->v == T('\n'); }
};

template <class T>
struct returns_and_marks_tag
{
	struct value_type {
		int returns;
		std::valarray<bool> marks;
		value_type() : returns(0) { marks.resize(128); }
		value_type &operator +=(const value_type &other) {
			returns += other.returns;
			marks |= other.marks;
			return *this;
		}
		value_type operator +(const value_type &other) {
			value_type ret(*this);
			ret += other;
			return ret;
		}
	};
	static value_type leaf_value() { return value_type(); }
	template<class Node>
	static value_type singleton_value(Node *n) {
		value_type ret;
		ret.returns = (n->v == T('\n'));
		return ret;
	}
};

template <class T>
struct text : public iv::list<T, returns_and_marks_tag<T>>
{
	typedef iv::list<T, returns_and_marks_tag<T>> parent_type;
	using typename parent_type::const_iterator;
	using typename parent_type::iterator;
	using parent_type::begin;
	using parent_type::end;
	using parent_type::root;
	void check() const
	{
		check(root());
	}

	void check(const_iterator i) const
	{
		if (!i.get())
			return;
		assert(i.stat()['\n'] == i.left().stat()['\n'] + (*i == T('\n')) + i.right().stat()['\n']);
		if (i.left().get()) {
			assert(i.left().parent() == i);
			check(i.left());
		}
		if (i.right().get()) {
			assert(i.right().parent() == i);
			check(i.right());
		}
	}

	int lines() const
	{
		return root().stat().returns;
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
				ret += j.left().stat().returns;
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
		if (root().stat().returns <= index)
			return line(lines() - 1);
		const_iterator ret = root();
		while (ret.left().stat().returns != index) {
			if (ret.left().stat().returns < index) {
				index -= ret.left().stat().returns + (*ret == T('\n'));
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
		this->clear();
		for (; begin != end; ++begin) {
			switch (*begin) {
			case '\t':
				for (int i = 0; i < 8; i++)
					this->push_back(' ');
				break;
			default:
				this->push_back(*begin);
				break;
			}
		}
	}
};

#endif // IV_TEXT_H
