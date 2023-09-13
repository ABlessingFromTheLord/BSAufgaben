CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700
.PHONY: all clean

all: clash

clash: clash.o plist.o
	$(CC) $(CFLAGS) -o clash clash.o plist.o

plist.o: plist.c plist.h
	$(CC) $(CFLAGS) -c plist.c

clash.o: clash.c plist.h
	$(CC) $(CFLAGS) -c clash.c

clean:
	rm -f clash.o
	rm -f plist.o
	rm -f main.o

