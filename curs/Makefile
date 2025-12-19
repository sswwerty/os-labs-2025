CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Iinclude

all: server client

server: src/server.c src/server_main.c src/game.c
	$(CC) $(CFLAGS) $^ -o server

client: src/client.c src/client_main.c src/game.c
	$(CC) $(CFLAGS) $^ -o client

clean:
	rm -f server client
