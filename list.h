#ifndef IV_LIST_H
#define IV_LIST_H

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

namespace iv
{

namespace internal
{

template <class T, class StatTag>
struct tree;

template <class T, class StatTag>
struct tree_base
{
	tree<T,StatTag> *l, *r; // left, right
	tree_base *p; // parent

	tree_base(tree<T,StatTag> *_l, tree<T,StatTag> *_r)
		: l(_l), r(_r)
	{
	}

	void resurrect();
};

template <class T, class StatTag>
struct tree_head : public tree_base<T,StatTag>
{
	tree_head(tree<T,StatTag> *_root = nullptr)
		: tree_base<T,StatTag>(_root, _root)
	{
	}
	tree<T,StatTag> *root() const
	{
		return this->l;
	}
};

template <class T, class StatTag>
struct tree : public tree_base<T,StatTag>
{
	T v; // contained value
	int h; // height
	typename StatTag::value_type s; // stat
	int size_;

	int size() const
	{
		if (this == nullptr)
			return 0;
		else
			return size_;
	}

	int height() const
	{
		if (this == nullptr)
			return 0;
		else
			return h;
	}

	typename StatTag::value_type stat() const
	{
		if (this == nullptr)
			return StatTag::leaf_value();
		else
			return s;
	}

	tree(tree *_l, const T &_v, tree *_r)
		: tree_base<T,StatTag>(_l, _r),
		v(_v),
		h(std::max(this->l->height(), this->r->height()) + 1),
		s(this->l->stat() + StatTag::singleton_value(this)
		  + this->r->stat()),
		size_(this->l->size() + 1 + this->r->size())
	{
		if (this->l) {
			//if (this->l->p)
				//this->l->pointer() = nullptr;
			this->l->p = this;
		}
		if (this->r) {
			//if (this->r->p)
				//this->r->pointer() = nullptr;
			this->r->p = this;
		}
	}

	template <class RandIt>
	static tree *construct(RandIt b, RandIt e)
	{
		if (b == e)
			return nullptr;
		int n = e - b;
		int c = n - 1;
		int l = c / 2;
		return new tree(construct(b, b + l), b[l], construct(b + l + 1, e));
	}

	tree *add_min(const T &x, tree *&inserted);
	tree *add_max(const T &x, tree *&inserted);
	tree *add_min(const T &x) { tree *inserted; return this->add_min(x, inserted); }
	tree *add_max(const T &x) { tree *inserted; return this->add_max(x, inserted); }
	T min() {
		assert(this != nullptr);
		tree *i = this;
		while (i->l != nullptr)
			i = i->l;
		return i->v;
	}
	T max() {
		assert(this != nullptr);
		tree *i = this;
		while (i->r != nullptr)
			i = i->r;
		return i->v;
	}
	tree *remove_min()
	{
		if (this->l == nullptr)
			return this->r;
		else
			return balance(this->l->remove_min(), this->v, this->r);
	}
	tree *remove_max()
	{
		if (this->r == nullptr)
			return this->l;
		else
			return balance(this->l, this->v, this->r->remove_max());
	}
	tree *insert(tree *before, const T &x, tree *&inserted);
	tree *erase(tree *it);
	tree *replace(tree *srcparent, tree * const &src, tree *dst);
	tree *&pointer() {
		if (this == this->p->l)
			return this->p->l;
		else if (this == this->p->r)
			return this->p->r;
		else
			throw std::runtime_error("no such child");
	}
	std::string str() const {
		if (this == nullptr)
			return "";
		return this->l->str() + std::string{this->v} + this->r->str();
	}
protected:
	static tree *join(tree *l, const T &v, tree *r);
	static tree *balance(tree *l, const T &v, tree *r);
	static tree *merge(tree *l, tree *r);
};

template <class T, class StatTag>
void tree_base<T,StatTag>::resurrect()
{
	//std::cerr << "Resurrecting " << this << std::endl;
	if (l) {
		l->p = this;
		l->resurrect();
	}
	if (r) {
		r->p = this;
		r->resurrect();
	}
}

/*
    let bal l v r =
      let hl = match l with Empty -> 0 | Node {h} -> h in
      let hr = match r with Empty -> 0 | Node {h} -> h in
      if hl > hr + 2 then begin
	match l with
	  Empty -> invalid_arg "Set.bal"
	| Node{l=ll; v=lv; r=lr} ->
	    if height ll >= height lr then
	      create ll lv (create lr v r)
	    else begin
	      match lr with
		Empty -> invalid_arg "Set.bal"
	      | Node{l=lrl; v=lrv; r=lrr}->
		  create (create ll lv lrl) lrv (create lrr v r)
	    end
      end else if hr > hl + 2 then begin
	match r with
	  Empty -> invalid_arg "Set.bal"
	| Node{l=rl; v=rv; r=rr} ->
	    if height rr >= height rl then
	      create (create l v rl) rv rr
	    else begin
	      match rl with
		Empty -> invalid_arg "Set.bal"
	      | Node{l=rll; v=rlv; r=rlr} ->
		  create (create l v rll) rlv (create rlr rv rr)
	    end
      end else
	Node{l; v; r; h=(if hl >= hr then hl + 1 else hr + 1)}
*/

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::balance(tree<T,StatTag> *l, const T &v, tree<T,StatTag> *r)
{
	if (l->height() > r->height() + 2) {
		if (l->l->height() >= l->r->height()) {
			auto right = new tree(l->r, v, r);
			return new tree(l->l, l->v, right);
		} else {
			auto left = new tree(l->l, l->v, l->r->l);
			auto right = new tree(l->r->r, v, r);
			return new tree(left, l->r->v, right);
		}
	} else if (r->height() > l->height() + 2) {
		if (r->r->height() >= r->l->height()) {
			auto left = new tree(l, v, r->l);
			return new tree(left, r->v, r->r);
		} else {
			auto left = new tree(l, v, r->l->l);
			auto right = new tree(r->l->r, r->v, r->r);
			return new tree(left, r->l->v, right);
		}
	} else
		return new tree(l, v, r);
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::merge(tree<T,StatTag> *l, tree<T,StatTag> *r)
{
	if (l == nullptr)
		return r;
	else if (r == nullptr)
		return l;
	else
		return balance(l, r->min(), r->remove_min());
}


template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::add_min(const T &x, tree<T,StatTag> *&inserted)
{
	if (this == nullptr)
		return inserted = new tree(nullptr, x, nullptr);
	else
		return balance(this->l->add_min(x, inserted), v, this->r);
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::add_max(const T &x, tree<T,StatTag> *&inserted)
{
	if (this == nullptr)
		return inserted = new tree(nullptr, x, nullptr);
	else
		return balance(this->l, v, this->r->add_max(x, inserted));
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::insert(tree<T,StatTag> *before, const T &x, tree<T,StatTag> *&inserted)
{
	return replace(before, before->l, before->l->add_max(x, inserted));
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::erase(tree<T,StatTag> *it)
{
	if (it == this)
		return merge(this->l, this->r);
	tree *i = it;
	while (i->p != this)
		i = static_cast<internal::tree<T, StatTag> *>(i->p);
	if (i == this->l) {
		return balance(this->l->erase(it), this->v, this->r);
	} else if (i == this->r) {
		return balance(this->l, this->v, this->r->erase(it));
	} else
		throw std::runtime_error("no child");
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::replace(tree<T,StatTag> *srcparent, tree<T,StatTag> * const &src, tree<T,StatTag> *dst)
{
	if (this == src) {
		return dst;
	}
	if (&src == &srcparent->r)
		return replace(static_cast<tree<T,StatTag> *>(srcparent->p), srcparent->pointer(), join(srcparent->l, srcparent->v, dst));
	else if (&src == &srcparent->l)
		return replace(static_cast<tree<T,StatTag> *>(srcparent->p), srcparent->pointer(), join(dst, srcparent->v, srcparent->r));
	else
		throw std::runtime_error("src has no parent");
}

template <class T, class StatTag>
tree<T,StatTag> *tree<T,StatTag>::join(tree<T,StatTag> *l, const T &v, tree<T,StatTag> *r)
{
	if (l == nullptr)
		return r->add_min(v);
	else if (r == nullptr)
		return l->add_max(v);
	else if (l->h > r->h + 2)
		return balance(l->l, l->v, join(l->r, v, r));
	else if (r->h > l->h + 2)
		return balance(join(l, v, r->l), r->v, r->r);
	else
		return new tree<T,StatTag>(l, v, r);
}

} // namespace iv::internal

template <class T, class StatTag>
class list;

template <class T, class StatTag>
class list_const_iterator : public std::iterator<std::random_access_iterator_tag, T>
{
	const list<T,StatTag> *lst;
	const internal::tree_base<T,StatTag> *ptr;
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = const T;
	using difference_type = int;
	using pointer = const T *;
	using reference = const T &;

	explicit list_const_iterator(const list<T,StatTag> *_lst = NULL,
	                             const internal::tree_base<T,StatTag> *node = nullptr)
		: lst(_lst), ptr(node)
	{
	}

	const internal::tree_base<T,StatTag> *get() const { return ptr; };
	list_const_iterator left()
	{
		return list_const_iterator(lst, this->get()->l);
	}
	list_const_iterator right()
	{
		return list_const_iterator(lst, this->get()->r);
	}
	list_const_iterator parent()
	{
		return list_const_iterator(lst, this->get()->p);
	}
	bool is_root() const
	{
		return parent() == *this;
	}
	int height() const
	{
		return static_cast<const internal::tree<T,StatTag> *>(this->get())->height();
	}
	int size() const
	{
		return static_cast<const internal::tree<T,StatTag> *>(this->get())->size();
	}
	typename StatTag::value_type stat() const
	{
		return static_cast<const internal::tree<T,StatTag> *>(this->get())->stat();
	}

	bool operator ==(const list_const_iterator &other)
	{
		return this->ptr == other.ptr;
	}
	bool operator !=(const list_const_iterator &other)
	{
		return this->ptr != other.ptr;
	}

	list_const_iterator &operator ++()
	{
		if (right().get()) {
			*this = right();
			while (left().get())
				*this = left();
		} else {
			while (*this != parent().left())
				*this = parent();
			*this = parent();
		}
		return *this;
	}
	list_const_iterator &operator --()
	{
		if (left().get()) {
			*this = left();
			while (right().get())
				*this = right();
		} else {
			while (*this != parent().right())
				*this = parent();
			*this = parent();
		}
		return *this;
	}
	list_const_iterator operator ++(int)
	{
		auto ret = *this;
		++*this;
		return ret;
	}
	list_const_iterator operator --(int)
	{
		auto ret = *this;
		--*this;
		return ret;
	}

	list_const_iterator operator +(int n)
	{
		list_const_iterator ret = *this;
		ret += n;
		return ret;
	}
	list_const_iterator &operator +=(int n)
	{
		*this = lst->iter_at(lst->index(*this) + n);
		return *this;
	}
	int operator -(list_const_iterator other)
	{
		return lst->index(*this) - lst->index(other);
	}

	reference operator *()
	{
		return static_cast<const internal::tree<T,StatTag> *>(this->ptr)->v;
	}
	pointer operator ->()
	{
		return &**this;
	}
	bool non_leaf() const
	{
		return this->get() != NULL;
	}
};

template <class T, class StatTag>
class list_iterator : public list_const_iterator<T,StatTag>
{
	friend class list<T,StatTag>;
protected:
	list_iterator(list_const_iterator<T,StatTag> other) : list_const_iterator<T,StatTag>(other) { }
	internal::tree_base<T,StatTag> *get() { return const_cast<internal::tree_base<T,StatTag> *>(list_const_iterator<T,StatTag>::get()); }
public:
	list_iterator() { }
	using value_type = T;
	using pointer = value_type *;
	using reference = value_type &;
	reference operator *()
	{
		return const_cast<reference>(list_const_iterator<T,StatTag>::operator *());
	}
	pointer operator ->()
	{
		return const_cast<pointer>(list_const_iterator<T,StatTag>::operator ->());
	}
	list_iterator &operator ++()
	{
		list_const_iterator<T,StatTag>::operator ++();
		return *this;
	}
	list_iterator &operator --()
	{
		list_const_iterator<T,StatTag>::operator --();
		return *this;
	}
	list_iterator operator ++(int)
	{
		list_iterator ret = *this;
		list_const_iterator<T,StatTag>::operator ++();
		return ret;
	}
	list_iterator operator --(int)
	{
		list_iterator ret = *this;
		list_const_iterator<T,StatTag>::operator --();
		return ret;
	}
	list_iterator operator +(int n)
	{
		list_iterator ret = *this;
		ret += n;
		return ret;
	}
	list_iterator &operator +=(int n)
	{
		list_const_iterator<T,StatTag>::operator +=(n);
		return *this;
	}
};

template <class T, class StatTag>
class list
{
	internal::tree_head<T,StatTag> *head;
public:
	typedef internal::tree<T,StatTag> tree_node_type;
	typedef list_const_iterator<T,StatTag> const_iterator;
	typedef list_iterator<T,StatTag> iterator;

	list() : head(new internal::tree_head<T,StatTag>())
	{
	}

	template <class RandIt>
	list(RandIt b, RandIt e) : head(new internal::tree_head<T,StatTag>())
	{
		head->l = head->r = internal::tree<T,StatTag>::construct(b, e);
		head->l->p = head;
	}

	void clear()
	{
		head->l = head->r = nullptr;
	}

	const_iterator begin() const
	{
		const_iterator ret(this, head);
		while (ret.left().get())
			ret = ret.left();
		return ret;
	}

	const_iterator end() const
	{
		return const_iterator(this, head);
	}

	iterator begin()
	{
		return const_cast<const list<T,StatTag> *>(this)->begin();
	}

	iterator end()
	{
		return const_cast<const list<T,StatTag> *>(this)->end();
	}

	const_iterator root() const
	{
		return const_iterator(this, head->root());
	}

	void push_front(const T &v)
	{
		head->l = head->r = head->root()->add_min(v);
		head->l->p = head;
	}
	void push_back(const T &v)
	{
		insert(end(), v);
		/*head->l = head->r = head->root()->add_max(v);
		head->l->p = head;*/
	}

	void check(const_iterator it)
	{
		int lh = it.left().non_leaf() ? it.left().height() : 0;
		int rh = it.right().non_leaf() ? it.right().height() : 0;
		assert(it.height() == std::max(lh, rh) + 1);
	}

	iterator insert(iterator before, const T &v)
	{
		internal::tree<T, StatTag> *inserted;
		if (before.get() == head)
			head->l = head->r = head->root()->add_max(v, inserted);
		else {
			head->l = head->r = head->root()->insert(static_cast<internal::tree<T, StatTag> *>(before.get()), v, inserted);
		}
		head->l->p = head;
		return iterator(const_iterator(this, inserted));
	}

	void erase(iterator it)
	{
		head->l = head->r = head->root()->erase(static_cast<internal::tree<T, StatTag> *>(it.get()));
		head->l->p = head;
	}

	int index(const_iterator i) const
	{
		int ret = 0;
		bool count = true;
		if (i == end())
			return i.left().size();
		for (; i != end(); i = i.parent()) {
			if (count)
				ret += i.left().size();
			count = (i == i.parent().right());
			if (count)
				ret += 1;
		}
		return ret;
	}

	const_iterator iter_at(int index) const
        {
                if (root().size() <= index)
                        return end();
                const_iterator ret = root();
                while (ret.left().size() != index) {
                        if (ret.left().size() < index) {
                                index -= ret.left().size() + 1;
                                ret = ret.right();
                        } else
                                ret = ret.left();
                }
                return ret;
        }


	int size() const
	{
		return head->l->size();
	}

protected:
	iterator make_iterator(const_iterator i)
	{
		return i;
	}
};

} // namespace iv

#endif // IV_LIST_H
