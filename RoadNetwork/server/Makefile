CC=g++
INCLUDES=-I ./include/
CFLAGS=-std=c++14 -O3
LIBS=-lpthread -lmetis

all: GPTree.o Position.o Car.o main.o
	$(CC) $(CFLAGS) $(INCLUDES) GPTree.o Car.o Position.o main.o -o server $(LIBS)

Position.o: Position.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c Position.cpp

Car.o: Car.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c Car.cpp

GPTree.o: ./include/GPTree.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ./include/GPTree.cpp

main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp

clean:
	rm *.o server
