CFLAGS=-g -fno-delete-null-pointer-checks -Wall -std=c++11 -Wno-nonnull-compare

iv: Makefile iv.cpp config.cpp handle_command.cpp list.h text.h
	g++ ${CFLAGS} -g -o $@ `pkg-config --cflags --libs ncursesw`  iv.cpp -pg -no-pie

iv.opt: Makefile iv.cpp config.cpp handle_command.cpp list.h text.h
	g++ ${CFLAGS} -O2 -o $@ `pkg-config --cflags --libs ncursesw`  iv.cpp

.PHONY: test
