CC = gcc
CFLAGS = -g -Wall -o
TARGETS = dirtree

dirtree: main.c dirtree.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf *.o *.swp $(TARGETS)
