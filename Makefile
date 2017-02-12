all: prog

prog: main.c chip.c chip.h 
	gcc -Wall main.c chip.c -o prog -I /usr/include/SDL/ `sdl-config --cflags --libs` -std=c99 

clean:
	rm prog
