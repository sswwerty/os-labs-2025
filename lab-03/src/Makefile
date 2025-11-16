CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic

all: parent child

parent: parent.c
	$(CC) $(CFLAGS) -o parent parent.c

child: child.c
	$(CC) $(CFLAGS) -o child child.c

clean:
	rm -f parent child shared.dat