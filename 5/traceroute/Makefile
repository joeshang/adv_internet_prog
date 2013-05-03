CC = gcc
CFLAGS = -g -Wall -o
TARGETS = mytrace

all: $(TARGETS)

mytrace: traceroute.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf *.o *.swp $(TARGETS)
