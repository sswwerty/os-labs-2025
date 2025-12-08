CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O2
LDFLAGS = -shared

all: libimpl1.so libimpl2.so prog

libimpl1.so: impl1.c calc.h
	$(CC) $(CFLAGS) impl1.c -o libimpl1.so $(LDFLAGS) -lm

libimpl2.so: impl2.c calc.h
	$(CC) $(CFLAGS) impl2.c -o libimpl2.so $(LDFLAGS) -lm

# prog будет использовать direct calls в static режиме если линкуется с -limpl1
prog: prog.c calc.h
	$(CC) -Wall prog.c -L. -limpl1 -o prog -ldl -lm
	@echo "Compiled prog (linked with libimpl1). To run dynamic mode: ./prog dynamic"

clean:
	rm -f *.so prog

.PHONY: all clean