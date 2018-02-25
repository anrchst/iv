CFLAGS=-g -Wall -std=gnu++11 -Wno-nonnull-compare

iv: Makefile iv.cpp config.cpp handle_command.cpp chunk.h buffer.h
	g++ ${CFLAGS}  -o $@ `pkg-config --cflags --libs ncursesw`  iv.cpp

.PHONY: test
