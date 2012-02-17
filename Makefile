CC=gcc
PKG_CONFIG=$(shell pkg-config --libs --cflags opencv)
LIBS=-I/usr/include/opencv -lcv -lhighgui -lm 
CFLAGS=-O3 -funroll-loops -Wall -Wno-unused-function -Wno-unused-value -std=gnu99 

all: mandelbrot

mandelbrot: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

nopng: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) $^ -lm -o mandelbrot

clean:
	rm mandelbrot
