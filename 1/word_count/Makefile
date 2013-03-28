CC = gcc
CFLAGS = -g -Wall -o
TARGETS = word_count

word_count: main.c handle_word.c darray.c 
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf *.swp *.o $(TARGETS)
