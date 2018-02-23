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
struct text : public iv::list<T, subtree_size_tag>
{
	typedef iv::list<T, subtree_size_tag> parent_type;
	using typename parent_type::const_iterator;
	using typename parent_type::iterator;
	using parent_type::begin;
	using parent_type::end;
	using parent_type::root;
	int index(const_iterator i)
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

	void check() const
	{
		check(root());
	}

	void check(const_iterator i) const
	{
		if (!i)
			return;
		assert(i.stat() == i.left().stat() + 1 + i.right().stat());
		if (i.left()) {
			assert(i.left().parent() == i);
			check(i.left());
		}
		if (i.right()) {
			assert(i.right().parent() == i);
			check(i.right());
		}
	}

	const_iterator upper_bound(int index) const
	{
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
	iterator upper_bound(int index)
	{
		return this->make_iterator(const_cast<const text *>(this)->upper_bound(index));
	}
};

#endif // IV_TEXT_H
