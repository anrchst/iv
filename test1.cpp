#include <cassert>
#include <iostream>
#include <iterator>
#include <string>
#include "text.h"

int main(int argc, char **argv)
{
	text<char> t;
	t.push_back('a');
	t.push_back('b');
	t.push_back('c');
	t.push_back('i');
	t.push_back('j');
	t.marks["_"] = 3;
	for (int i = 0; i < std::atoi(argv[1]); i++) {
		t.insert('d' + i % 5);
		std::cout << "===" << std::endl;
		std::cout << t.marks["_"] << std::endl;
		for (auto k = t.begin(); k != t.end(); ++k) {
			t.check(k);
			std::cout << k.height() << " " << *k << std::endl;
		}
	}
	auto j = t.begin();
	assert(*j++ == 'a');
	assert(*j++ == 'b');
	assert(*j++ == 'c');
	for (int i = 0; i < std::atoi(argv[1]); i++, j++) {
		std::cout << "!" << (*j) << std::endl;
		assert(*j == ('d' + i % 5));
	}
	std::cout << "==" << (*j) << std::endl;
	assert(*j++ == 'i');
	assert(*j++ == 'j');
	assert(j == t.end());
}
