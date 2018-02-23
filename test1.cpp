#include <cassert>
#include <string>
#include "text.h"

int main()
{
	std::string s = "abcde\nfgh\n";
	text<char> t;
	t.assign(s.begin(), s.end());
	assert(*t.begin() == 'a');
	assert(t.line(t.begin()) == 0);
	text<char>::iterator f = t.begin();
	std::advance(f, 6);
	assert(*f == 'f');
	assert(t.line(f) == 1);
}
