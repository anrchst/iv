#include <cassert>
#include <iterator>
#include <string>
#include "text.h"

/***********************
 * B R O K E N         *
 * *********************/
int main()
{
	std::string s = "abcdefgh\nABCDEFGH\n\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\nabcdefgh\nABCDEFGH\n";
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
		//std::cout << "!" << i << " " << t.line(f) << std::endl;
		assert(t.line(f) == 1);
		f++;
	}
	assert(t.line(f) == 2);
	assert(std::distance(t.begin(), t.line(0)) == 0);
	assert(*--f == '\n');
	assert(t.line(f) == 1);
	//std::cout << t.line(--t.end()) << std::endl;
	assert(t.line(--t.end()) == 10);
	assert(*t.line(0) == 'a');
	assert(*t.line(1) == 'A');
	assert(*t.line(2) == '\n');
	assert(*t.line(3) == 'a');
	t.check();
}
