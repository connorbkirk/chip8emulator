#ifndef CHIP_8
#define CHIP_8

#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
	
#define RESIZE_FACTOR 10
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64
#define KEY_SIZE 17

unsigned char memory[4096];//4kb memory
unsigned char v[16];//registers
unsigned short i;//instruction
unsigned short pc;//program counter

unsigned short stack[16];//stack
unsigned short sp;//stack pointer

int delay_timer;
int sound_timer;

unsigned char keys[16];
int keylocation;

unsigned char display[SCREEN_HEIGHT * SCREEN_WIDTH];

bool needsRedraw;
bool running;
bool waitKey;

SDL_Window *window;
SDL_Renderer *renderer;

void chip_init();
void chip_run();
void chip_load_file(char * file);
void chip_load_fontset();



int display_init();
void display_update();
void display_destroy();
void display_clear();
void display_draw(int x, int y, int w, int h, bool fill);
int display_handle_keys();
void display_handle_input();

static const unsigned char fontset[5*16] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

enum keys{
	KEY_CLEAR = -3,
	KEY_NO = -2,
	KEY_QUIT = -1,
	KEY_ESCAPE = -1,
	KEY_ONE = 1,
	KEY_TWO = 2,
	KEY_THREE = 3,
	KEY_FOUR = 0xC,
	KEY_Q = 4,
	KEY_W = 5,
	KEY_E = 6,
	KEY_R = 0xD,
	KEY_A = 7,
	KEY_S = 8,
	KEY_D = 9,
	KEY_F = 0xE,
	KEY_Z = 0xA,
	KEY_X = 0x0,
	KEY_C = 0xB,
	KEY_V = 0xF
};

static const int key_key[KEY_SIZE] = {
	SDL_SCANCODE_ESCAPE, SDL_SCANCODE_1, SDL_SCANCODE_2,
	SDL_SCANCODE_3, SDL_SCANCODE_4, SDL_SCANCODE_Q,
	SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
	SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D,
	SDL_SCANCODE_F, SDL_SCANCODE_Z, SDL_SCANCODE_X,
	SDL_SCANCODE_C, SDL_SCANCODE_V
};

static const int key_value[KEY_SIZE] = {
	KEY_QUIT, KEY_ONE, KEY_TWO,
	KEY_THREE, KEY_FOUR, KEY_Q,
	KEY_W, KEY_E, KEY_R,
	KEY_A, KEY_S, KEY_D,
	KEY_F, KEY_Z, KEY_X,
	KEY_C, KEY_V	
};


#endif
