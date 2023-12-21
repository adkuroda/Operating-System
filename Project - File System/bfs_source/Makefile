CC=gcc
CFLAGS=-Wall

clean:
	rm -rf *.o main

all: main

main: main.o p5test.o fs.o errors.o deb.o bio.o bfs.o
	$(CC) $(CFLAGS) -o main main.o p5test.o fs.o errors.o deb.o bio.o bfs.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

bfs.o: bfs.c bfs.h
	$(CC) $(CFLAGS) -c bfs.c

p5test.o: p5test.c p5test.h
	$(CC) $(CFLAGS) -c p5test.c

fs.o: fs.c fs.h
	$(CC) $(CFLAGS) -c fs.c

errors.o: errors.c errors.h
	$(CC) $(CFLAGS) -c errors.c

deb.o: deb.c deb.h
	$(CC) $(CFLAGS) -c deb.c

bio.o: bio.c bio.h
	$(CC) $(CFLAGS) -c bio.c

