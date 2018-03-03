#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include "text.h"

int main()
{
	text<char> t;
	t.push_back('a');
	t.push_back('b');
	t.push_back('c');
	t.marks["_"] = 3;
	for (int i = 0; i < 5; i++) {
		t.insert('d' + i % 5);
		std::cout << "===" << std::endl;
		for (auto k = t.begin(); k != t.end(); k++)
			std::cout << k.height() << " " << *k << std::endl;
	}
	auto j = t.begin();
	assert(*j++ == 'a');
	assert(*j++ == 'b');
	assert(*j++ == 'c');
	for (int i = 0; i < 10000; i++, j++)
		assert(*j >= 'd');
	assert(j == t.end());
}
