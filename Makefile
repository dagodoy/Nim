CC=g++
CFLAGS=-g -I.
DEPS = Socket.h Nim.h
OBJ = Socket.o Nim.o
LIBS=-lpthread

%.o: %.cc $(DEPS)
	$(CC) -g -c -std=c++11 -o $@ $< $(CFLAGS)

all: ns nc

ns: $(OBJ) NimServer.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)

nc: $(OBJ) NimClient.o
	g++ -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean

clean:
	rm -f *.o ns nc

