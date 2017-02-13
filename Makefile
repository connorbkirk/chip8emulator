CFLAGS := gcc -Wall -g $(shell sdl2-config --cflags)
LDFLAGS += $(shell sdl2-config --libs)

all: prog

prog: main.c chip.c chip.h 
	$(CFLAGS)  main.c chip.c -o prog $(LDFLAGS) 

clean:
	rm prog
