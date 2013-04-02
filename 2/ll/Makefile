CC = gcc
CFLAGS = -g -Wall -o
TARGETS = ll

ll: main.c ll.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf $(TARGETS) *.swp *.o
