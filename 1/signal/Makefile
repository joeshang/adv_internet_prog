CC = gcc
CFLAGS = -g -Wall -o
TARGETS = signal

signal: signal.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf $(TARGETS) *.o
