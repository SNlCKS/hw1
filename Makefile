all: waterfall

waterfall: waterfall.cpp fonts.h
	g++ waterfall.cpp libggfonts.a -Wall -Wextra -owaterfall -lX11 -lGL -lGLU -lm -lrt

clean:
	rm -f waterfall
	rm -f *.o

