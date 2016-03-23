CC=g++
CFLAGS=`pkg-config opencv --cflags` -Os

LIBS=`pkg-config opencv --libs`

all: main.cpp
	$(CC) $(CFLAGS) main.cpp $(LIBS) -o main

clean:
	rm main
