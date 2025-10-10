CC = gcc
CFLAGS = -O2 -Wall -Wextra -pthread
TARGET = qsort_mt
SRC = main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) -n 200000 -m 4 -v

clean:
	rm -f $(TARGET)