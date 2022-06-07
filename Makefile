.POSIX:
.SUFFIXES:

CC = clang
CFLAGS = -std=c99 -pedantic -Wall -Wextra

SOURCES = cl-example.c cl.c
HEADERS = cl.h

cl-example: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LDLIBS)

clean:
	rm -f cl-example
