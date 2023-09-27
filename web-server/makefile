CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Wshadow
CFLAGS+=-O3
#CFLAGS+=-g -fsanitize=address -O0
LDLIBS=-lpthread

web-server: server.o file.o main.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

main.o: main.c server.h def.h
	$(CC) $(CFLAGS) -c $< -o $@

server.o: server.c server.h def.h
	$(CC) $(CFLAGS) -c $< -o $@

file.o: file.c file.h def.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf *.o web-server
