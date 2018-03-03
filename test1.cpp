#include <cassert>
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
	for (int i = 0; i < 5; i++)
		t.insert('d' + i % 5);
	auto j = t.begin();
	assert(*j++ == 'a');
	assert(*j++ == 'b');
	assert(*j++ == 'c');
	for (int i = 0; i < 10000; i++, j++)
		assert(*j >= 'd');
	assert(j == t.end());
}
