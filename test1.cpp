#include <cassert>
#include <string>
#include "text.h"

int main()
{
	std::string s = "abcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\n";
	text<char> t;
	t.assign(s.begin(), s.end());
	assert(*t.begin() == 'a');
	assert(t.line(t.begin()) == 0);
	text<char>::iterator f = t.begin();
	for (int i = 0; i < 9; i++) {
		assert(t.line(f) == 0);
		f++;
	}
	for (int i = 0; i < 9; i++) {
		assert(t.line(f) == 1);
		f++;
	}
	assert(*--f == '\n');
	assert(t.line(f) == 1);
	t.check();
}
