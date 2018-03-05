CFLAGS=-g -Wall -std=c++11 -Wno-nonnull-compare

iv: Makefile iv.cpp config.cpp handle_command.cpp list.h text.h
	g++ ${CFLAGS}  -o $@ `pkg-config --cflags --libs ncursesw`  iv.cpp -pg -no-pie

test: test1
	./test1 1000

test1: test1.cpp list.h text.h Makefile
	g++ ${CFLAGS} -o $@ test1.cpp

.PHONY: test
