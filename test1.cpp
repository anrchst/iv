#include <cassert>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "list.h"

int main(int argc, char **argv)
{
	std::cout << "Test1" << std::endl;
	iv::list<int> l;
	int n = std::atoi(argv[1]);
	for (int i = 0; i < n; i++) {
		l.push_back(i);
		iv::list<int>::const_iterator it = l.begin();
		for (int j = 0; j <= i; j++) {
			if (i % 200 == 0 && j % 200 == 0)
				std::cout << i << " " << j << std::endl;
			assert(*it == j);
			assert(l.index(it) == j);
			++it;
		}
	}
	assert(l.index(l.end()) == n);
	return 0;
}
