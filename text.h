#ifndef IV_TEXT_H
#define IV_TEXT_H

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
struct text : public iv::list<T, returns_tag<T>>
{
	typedef iv::list<T, returns_tag<T>> parent_type;
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
		if (!i)
			return;
		assert(i.stat() == i.left().stat() + (*i == T('\n')) + i.right().stat());
		if (i.left()) {
			assert(i.left().parent() == i);
			check(i.left());
		}
		if (i.right()) {
			assert(i.right().parent() == i);
			check(i.right());
		}
	}

	int line(const_iterator i) const
	{
		int ret = 0;
		bool count = true;
		if (i == end())
			return i.left().stat();
		for (; i != end(); i = i.parent()) {
			if (count)
				ret += i.left().stat();
			count = (i == i.parent().right());
			if (count)
				ret += 1;
		}
		return ret;
	}

	const_iterator line(int index) const
	{
		if (index < 0)
			index = 0;
		if (root().stat() <= index)
			return end();
		const_iterator ret = root();
		while (ret.left().stat() != index) {
			if (ret.left().stat() < index) {
				index -= ret.left().stat() + 1;
				ret = ret.right();
			} else
				ret = ret.left();
		}
		return ret;
	}
	iterator line(int index)
	{
		return this->make_iterator(const_cast<const text *>(this)->line(index));
	}
};

#endif // IV_TEXT_H
