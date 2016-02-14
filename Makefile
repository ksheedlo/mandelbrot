CC=gcc
PKG_CONFIG=$(shell pkg-config --libs --cflags opencv)
LIBS=-lm 
CFLAGS=-O3 -funroll-loops -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-value -Werror -std=gnu99 

all: mandelbrot

mandelbrot: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) -DPNG_CAPABLE mandelbrot.c $(PKG_CONFIG) $(LIBS) -o $@ 

nopng: mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) $^ -lm -o mandelbrot

clean:
	rm mandelbrot
