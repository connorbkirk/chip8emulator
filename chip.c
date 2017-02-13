#include "chip.h"
void chip_init(){
	i = 0x0;
	pc = 0x200;
	sp = 0;
	
	delay_timer = 0;
	sound_timer = 0;

	needsRedraw = false;

	chip_load_fontset();
}

void chip_load_fontset(){
	for(int i = 0; i < 5*16; i++)
		memory[i+0x50] = fontset[i];
}

void chip_load_file(char * file){
	FILE *fp;
	size_t read;

	//open file in read-only mode	
	fp = fopen(file, "r");
	
	if(!fp){
		fprintf(stderr, "Could not open file: %s\n", file);
		exit(EXIT_FAILURE);
	}
	
	read = fread(memory+0x200, 1, 4096-0x200, fp);
	if( read == 0){
		fprintf(stderr, "File %s could not be read\n", file);
		exit(EXIT_FAILURE);	
	}	

/*	for(int i = 0; i < 4096; i++)
		printf("%02x\n", memory[i]);
*/	
	fclose(fp);
}

void chip_run(){
	unsigned short opcode, address;

	//fetch opcode
	opcode = memory[pc] << 8 | memory[pc+1];
	printf("Opcode: %02x\n", opcode);
	
	//decode opcode
	switch(opcode & 0xF000){//mask to get first nibble
		case 0x1000://1NNN - jump to address NNN
			address = opcode & 0x0FFF;//bitmap
			pc = address;	
			break;

		case 0x2000://2NNN - call subroutine at address NNN
			address = opcode & 0x0FFF;//bitmap to get last 3
		
			//store pc in stack	
			stack[sp++] = pc;

			pc = address;		
			break;

		case 0x3000://3XNN - skips the next instructions if VX == NN
			if( v[opcode&0x0F00] == (opcode&0x00FF) )
				pc+=4;
			else
				pc+=2;	
			break;
		
		case 0x6000://6XNN - set VX to NN
			v[opcode&0x0F00] = opcode&0x00FF;
			pc+=2;	
			break;
		
		case 0x7000://7XNN - add VX to NN
			v[opcode&0x0F00] += opcode&0x00FF;
			pc+=2;	
			break;	

		case 0x8000:
			switch(opcode & 0x000F){//mask to get last nibble
				case 0x0000://8XY0 - set VX to XY
					v[opcode&0x0f00]=v[opcode&0x00f0];
					break;
				default:
					printf("Unsupported opcode: %04x\n. System exit\n", opcode);
					exit(EXIT_FAILURE);	
					break;
			}

		case 0xA000://ANNN - set i to NNN
			address = opcode & 0xF000;
			i = address;
			pc+=2;
			break;

		case 0xD000: //DXYN - draw a sprite (X,Y) size (8,N) located at I
			pc+=2;	
			break;
		
		default:
			printf("Unsupported opcode: %04x. System exit\n", opcode);
			exit(EXIT_FAILURE);	
			break;
	}	

}

void removeDrawFlag(){
	needsRedraw = false;
}



int display_init(){
	printf("Starting display\n");
	if(SDL_Init(SDL_INIT_EVERYTHING) < 0) return 1;

	window = SDL_CreateWindow("Connor's Chip 8 Emulator",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if(window == NULL)
		return 1;

	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH/RESIZE_FACTOR,
		SCREEN_HEIGHT/RESIZE_FACTOR);

	return 0;
}

int display_update(){
	SDL_UpdateTexture(texture, NULL, display, SCREEN_WIDTH/RESIZE_FACTOR * sizeof(unsigned char *));

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	return 0;
}



