#ifndef IV_CHUNK_H
#define IV_CHUNK_H

#include <cassert>
#include <ext/rope>

/* several whole contigous lines */
struct chunk : public __gnu_cxx::rope<char>
{
	int chars;
	int lines;

	chunk() : chars(0), lines(0) { }
	explicit chunk(__gnu_cxx::rope<char> other)
		: __gnu_cxx::rope<char>(other)
		, chars(0), lines(0)
	{
		for (auto i = begin(); i != end(); ++i) {
			{
				auto j = i;
				++j;
				if (j == end())
					assert(*i == '\n');
			}
			chars++;
			lines += (*i == '\n');
		}
	}
	void push_back(char c)
	{
		__gnu_cxx::rope<char>::push_back(c);
		chars++;
		lines += (c == '\n');
	}
};

#endif // IV_CHUNK_H
