CFLAGS = -Wall -g -std=c++11

all: main.cpp lrucache.h bplus.h diskmngr.h
	g++ ${CFLAGS} -o main main.cpp

clean:
	rm db

run:
	make clean
	make
	./main
