#include <cassert>
#include <iterator>
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
	assert(std::distance(t.begin(), t.line(0)) == 0);
	assert(*--f == '\n');
	assert(t.line(f) == 1);
	std::cout << t.line(--t.end()) << std::endl;
	assert(t.line(--t.end()) == 9);
	t.check();
}
