CC = gcc
CFLAGS = -g -Wall -o
TARGETS = trans_client file_server

all: $(TARGETS)

trans_client: trans_client.c utility.c
	$(CC) $(CFLAGS) $@ $^

file_server: file_server.c utility.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf *.o *.swp $(TARGETS)
