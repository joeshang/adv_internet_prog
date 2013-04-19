CC = gcc
CFLAGS = -g -Wall -o
TARGETS = chat_client chat_server_select chat_server_thread

all: $(TARGETS)

chat_client: chat_client.c
	$(CC) $(CFLAGS) $@ $^

chat_server_select: chat_server_select.c 
	$(CC) $(CFLAGS) $@ $^

chat_server_thread: chat_server_thread.c 
	$(CC) $(CFLAGS) $@ $^ -lpthread

.PHONY: clean
clean:
	rm -rf *.o *.swp $(TARGETS)
