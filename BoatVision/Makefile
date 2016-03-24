CC=g++
CFLAGS=`pkg-config opencv --cflags` -Ofast -march=native

LIBS=`pkg-config opencv --libs`

all: main.cpp
	$(CC) $(CFLAGS) main.cpp $(LIBS) -o main

clean:
	rm main
