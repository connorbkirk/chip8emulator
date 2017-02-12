#ifndef CHIP_8
#define CHIP_8

#include <stdbool.h>
#include <stdio.h>
#include <SDL/SDL.h>

#define RESIZE_FACTOR 10
#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH 640
#define BPP 4
#define SCREEN_DEPTH 10

unsigned char memory[4096];//4kb memory
unsigned char v[16];//registers
unsigned short i;//instruction
unsigned short pc;//program counter

unsigned short stack[16];//stack
unsigned short sp;//stack pointer

int delay_timer;
int sound_timer;

unsigned char keys[16];

unsigned char display[SCREEN_HEIGHT * SCREEN_WIDTH];

bool needsRedraw;


SDL_Surface *screen;
SDL_Event event;


void chip_init();
void chip_run();
void chip_load(char * file);

void removeDrawFlag();


int display_init();
int display_clear();
int display_draw(int x, int y, int c);
void display_setpx(int x, int y, unsigned char r, unsigned char g, unsigned char b);
int display_update();

#endif
