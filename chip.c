#include "chip.h"
void chip_init(){
	i = 0x0;
	pc = 0x200;
	sp = 0;
	
	delay_timer = 0;
	sound_timer = 0;

	needsRedraw = false;
}

void chip_load(char * file){
}

void chip_run(){
	//fetch opcode
	unsigned short opcode = memory[pc] << 8 | memory[pc+1];
	printf("Opcode: %d\n", opcode);
	
	//decode opcode
	switch(opcode & 0xF000){//look @ first nibble
		case 0x8000://need value from last nibble
			switch(opcode & 0x000F){
				case 0x0000:
					//8xy0
					v[opcode&0x0f00]=v[opcode&0x00f0];
				break;
				default:
					printf("Unsupported opcode. System exit\n");
					exit(EXIT_FAILURE);	
					break;
			}
		default:
			printf("Unsupported opcode. System exit\n");
			exit(EXIT_FAILURE);	
			break;
	}	

	//execute opcode
}

void removeDrawFlag(){
	needsRedraw = false;
}



int display_init(){
	
	printf("Starting display\n");
	if(SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

	if(!(screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, SDL_HWSURFACE))){
		SDL_Quit();
		return 1;
	}

	return 0;
}

int display_clear(){
	int x, y;

	for(y = 0; y < 32; y++){
		for(x = 0; x < 64; x++){
			display_draw(x,y,0);
		}
	}

	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_Flip(screen);

	return 0;
}

int display_draw(int x, int y, int c){
	int yw;
	int blocky;
	int blockx;
	x = x * RESIZE_FACTOR;
	y = y * RESIZE_FACTOR;

	if(c==1)
		c=128;
	else
		c=0;	

	if(SDL_MUSTLOCK(screen)){
		if(SDL_LockSurface(screen) < 0)
			return 1;	
	}

	for(blocky = 0; blocky < RESIZE_FACTOR; blocky++){
		yw = y*screen->pitch / BPP;
		for(blockx=0; blockx<RESIZE_FACTOR; blockx++){
			display_setpx(blockx+x, (blocky*screen->pitch/BPP) + yw, c, c, c);
		}	
	}
	return 0;
}

void display_setpx(int x, int y, unsigned char r, unsigned char g, unsigned char b){
	unsigned int *pixmem;
	unsigned int color;

	color = SDL_MapRGB(screen->format, r, g, b);
	
	pixmem = (unsigned int*)screen->pixels + y + x;
	*pixmem = color;
}

int display_update(){
	int x, y;
	display_clear();
	
	for(y = 0; y < SCREEN_HEIGHT/RESIZE_FACTOR; y++){
		for(x = 0; x < SCREEN_WIDTH/RESIZE_FACTOR; x++){
			if( display[x*SCREEN_WIDTH+y] != 0)
				display_draw(x,y,1);
		}
	}

	if(SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);
	SDL_Flip(screen);
	return 0;
}




