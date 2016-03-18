CC=g++
CFLAGS=`pkg-config opencv --cflags`

LIBS=`pkg-config opencv --libs`

all: main.cpp
	$(CC) $(CLFAGS) main.cpp $(LIBS) -o main

clean:
	rm main
