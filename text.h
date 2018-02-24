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
		if (!i.get())
			return;
		assert(i.stat() == i.left().stat() + (*i == T('\n')) + i.right().stat());
		if (i.left().get()) {
			assert(i.left().parent() == i);
			check(i.left());
		}
		if (i.right().get()) {
			assert(i.right().parent() == i);
			check(i.right());
		}
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
		if (root().stat() <= index)
			return end();
		const_iterator ret = root();
		while (ret.left().stat() != index) {
			if (ret.left().stat() < index) {
				index -= ret.left().stat() + (*ret == T('\n'));
				ret = ret.right();
			} else
				ret = ret.left();
		}
		while (ret != end() && *ret != T('\n'))
			--ret;
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
