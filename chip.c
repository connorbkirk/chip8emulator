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
		//fprintf(stderr, "File %s could not be read\n", file);
		exit(EXIT_FAILURE);	
	}	

/*	for(int i = 0; i < 4096; i++)
		printf("%02x\n", memory[i]);
*/	
	fclose(fp);
}

void chip_run(){
	unsigned short opcode, address, x, y, height, line;
	unsigned char _x, _y, px;
	
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
			printf("Calling %04x\n", address);	
			break;

		case 0x3000://3XNN - skips the next instructions if VX == NN
			if( v[ (opcode&0x0F00) >> 8 ] == (opcode&0x00FF) )
				pc+=4;
			else
				pc+=2;	
			break;
		
		case 0x6000://6XNN - set VX to NN
			v[ (opcode&0x0F00) >> 8 ] = opcode&0x00FF;
			pc+=2;
			printf("Setting v[%d] to %d\n", (opcode&0x0F00) >> 8, v[(opcode&0x0F00) >> 8]);	
			break;
		
		case 0x7000://7XNN - add VX to NN
			v[ (opcode&0x0F00) >> 8 ] += opcode&0x00FF;
			pc+=2;	
			printf("Adding %d to v[%d] = %d\n", opcode&0x00FF, (opcode&0x0F00) >> 8, v[(opcode&0x0F00) >> 8]);	
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
			address = opcode & 0x0FFF;
			i = address;
			pc+=2;
			printf("Set i to %04x\n", i);
			break;

		case 0xD000: //DXYN - draw a sprite (X,Y) size (8,N) located at I
			x = v[ (opcode & 0x0F00) >> 8 ];	
			y = v[ (opcode & 0x00F0) >> 4 ];	
			height = opcode & 0x000F;
		
			v[0xF] = 0;//collision flag
	
			for(_y = 0; _y < height; _y++){
			//	printf("Adding %d and %d\n", i, _x);
				line = memory[i + _y];	
				for(_x= 0; _x < 8; _x++){
					px = line & (0x80 >> _x);	
					if(px != 0){
						address = (y+_y)*SCREEN_WIDTH + (x+_x);
						
						if(display[address] == 1)
							v[0xF] = 1;
						
						display[address] ^= 1;	
					}
				}	
			}
			pc+=2;
			needsRedraw = true;	
			printf("Drawing @ v[%d]=%d, v[%d]=%d\n", (opcode&0x0F00)>>8, x, (opcode&0x00F0) >> 4, y);	
			break;
		
		default:
			printf("Unsupported opcode: %04x. System exit\n", opcode);
			exit(EXIT_FAILURE);	
			break;
	}	

}

int display_init(){
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	window = SDL_CreateWindow(
		"Connor's Chip 8 Emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH * RESIZE_FACTOR,
		SCREEN_HEIGHT * RESIZE_FACTOR,
		SDL_WINDOW_OPENGL);

	if(window == NULL)
		return 1;	
	
	renderer = SDL_CreateRenderer(
		window, 0, SDL_RENDERER_ACCELERATED);

	return 0;
}

void display_update(){
	display_clear();

	for(int y = 0; y < SCREEN_HEIGHT; y++){
		for(int x = 0; x < SCREEN_WIDTH; x++){
			display_draw(x,y,RESIZE_FACTOR, RESIZE_FACTOR,
				display[ (y*SCREEN_WIDTH)+x ] != 0 ? true : false);
		}	
	}

	SDL_RenderPresent(renderer);
}

void display_destroy(){
	if(renderer){
		SDL_DestroyRenderer(renderer);
		renderer = NULL;	
	}

	if(window){
		SDL_DestroyWindow(window);
		window = NULL;
	}

	SDL_Quit();
}

void display_clear(){
	SDL_RenderClear(renderer);
}

void display_draw(int x, int y, int w, int h, bool fill){
	SDL_SetRenderDrawColor(
		renderer,
		fill ? 0 : 255,
		fill ? 0 : 255,
		fill ? 0 : 255,
		255);

	SDL_RenderFillRect(renderer, &(SDL_Rect){
		.x = x * RESIZE_FACTOR,
		.y = y * RESIZE_FACTOR,
		.w = w,
		.h = h});

}
