CC = gcc
CFLAGS = -g -Wall -o
TARGETS = chat_client chat_server

all: $(TARGETS)

chat_client: chat_client.c
	$(CC) $(CFLAGS) $@ $^

chat_server: chat_server.c
	$(CC) $(CFLAGS) $@ $^

.PHONY: clean
clean:
	rm -rf *.o *.swp $(TARGETS)
