#include "chip.h"
void chip_init(){
	memset( memory, 0, sizeof(memory) );
	memset( v, 0, sizeof(v) );
	memset( keys, 0, sizeof(keys) );

	i = 0x0;
	pc = 0x200;
	sp = 0;
	
	delay_timer = 0;
	sound_timer = 0;

	needsRedraw = false;
	running = false;
	waitKey = false;
	keylocation = 0;
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
	printf("%02x: ", opcode);
	
	//decode opcode
	switch(opcode & 0xF000){//mask to get first nibble
		case 0x0000://Multi-case
			switch(opcode & 0x0FF){//mask to get last 2 nibbles
				case 0x00E0://00E0 - clear screen
					display_clear();
					memset(display, 0, sizeof(display));
					needsRedraw = true;
					pc+=2;
					break;
				
				case 0x00EE://00EE - return from subroutine
					sp--;	
					pc = stack[sp] + 2;
					printf("Returning to %04x\n", pc);	
					break;
			
				default: 
					printf("Unsupported opcode: %04x\n. System exit\n", opcode);
                                        exit(EXIT_FAILURE);			
					break;
			}
			break;
			
		case 0x1000://1NNN - jump to address NNN
			address = opcode & 0x0FFF;//bitmap
			pc = address;
			printf("Jumping to %04x\n", pc);	
			break;

		case 0x2000://2NNN - call subroutine at address NNN
			address = opcode & 0x0FFF;//bitmap to get last 3
		
			//store pc in stack	
			stack[sp++] = pc;

			pc = address;		
			printf("Calling %04x\n", address);	
			break;

		case 0x3000://3XNN - skips the next instructions if VX == NN
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00FF);//NN	

			if( v[x] == y ){
				printf("Skipping next instruction (v[%d] == %d\n", x, y);
				pc+=4;
			}
			else{
				printf("Not skipping next instruction (v[%d] != %d)\n", x, y); 
				pc+=2;	
			}
			break;
	
		case 0x4000://4XNN: skip next instruction if v[x] != nn
			x = (opcode & 0xF00) >> 8;
			y = opcode & 0x0FF;//nn

			if( v[x] != y ){
				printf("Skipping next instruction since v[%d] != %d\n", x, y);
				pc+=4;
			}
			else{
				printf("Not skipping next instruction since v[%d] == %d\n", x, y); 
				pc+=2;
			}
			break;

		case 0x5000://5XY0 - skip next instruction is v[x] == v[y]
			x = (opcode & 0xF00) >> 8;
			y = (opcode & 0x0F0) >> 4;

			if( v[x] == v[y] ){
				printf("Skipping next instruction since v[%d] == v[%d]\n", x, y);
				pc+=4;
			}else{
				printf("Not skipping next instruction since v[%d] != v[%d]\n", x, y);
				pc+=2;
			}			
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
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					v[x] = v[y];
					printf("Set v[%d] to v[%d] = %d\n", x, y, v[x]);
					pc+=2;
					break;

				case 0x0001://8XY1 - set v[x] to v[x] or v[y]
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					v[x] = (v[x] | v[y]) & 0xFF;
					printf("Setting v[%d] = v[%d] | v[%d]\n", x, x, y);	
					pc+=2;
					break;

				case 0x0002://8XY2 - set v[X] to v[X] & v[Y]
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					v[x] = v[x] & v[y];
					printf("Set v[%d] to v[%d] & v[%d] = %d\n", x, x, y, v[x]);
					pc+=2;
					break;
				
				case 0x0003://8XY3 - set v[x] to v[x] xor v[y]
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					v[x] = (v[x] ^ v[y]) & 0xFF;
					printf("Setting v[%d] = v[%d] ^ v[%d]\n", x, x, y);	
					pc+=2;
					break;

				case 0x0004://8XY4 - adds v[y] to v[x]. v[f] is set to 1 when overflow. v[f] is set to 0 if no overflow
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					if(v[y] > 255 - v[x])
						v[0xF] = 1;
					else
						v[0xF] = 0;
					v[x] = (v[x]+v[y]) & 0xFF;
					printf("Adding v[%d] to v[%d] = %d\n", y, x, v[x]);
					pc+=2;	
					break;	

				case 0x0005://8VX5 - v[x] -= v[y]. v[f] is set to 0 when there is a borrow. else it is set to 1
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					
					printf("v[%d] -= v[%d]\n", x, y);	
					if(v[x] > v[y]){
						v[0xF] = 1;
						printf("No borrow\n");
					}else{
						v[0xF] = 0;
						printf("Borrow\n");
					}
					v[x] = ( v[x] - v[y] ) & 0xFF;			
					pc+=2;	
					break;

				case 0x0006://8XY6 - shift v[x] right 1, v[F] is set to the least significant bit of v[X]
					x = (opcode & 0xF00) >> 8;
					v[0xF] = (v[x] & 0x1);	
					v[x] = v[x] >> 1;
					printf("Shift v[%d] >> 1 and v[F] to LSB of v[%d]\n", x, x);	
					pc+=2;	
					break;
				
				case 0x0007://8XY7 - sets v[x] to v[y] - v[x]. v[f] is set to 0 when there is a borrow. and 1 when there isnt
					x = (opcode & 0x0f00) >> 8;
					y = (opcode & 0x00f0) >> 4;
					if(v[x] > v[y])
						v[0xF] = 0;
					else
						v[0xF] = 1;
					v[x] = (v[y]-v[x]) & 0xFF;
					printf("Setting v[%d] to v[%d] - v[%d] = %d\n", x, y, x, v[x]);
					pc+=2;	
					break;	

				case 0x000E://8XYE - shift v[x] left by 1. v[f] is set to the value of the most significant bit of v[x] before shift
					x = (opcode & 0xF00) >> 8;
					v[0xF] = (v[x] & 0x80);	
					v[x] = v[x] << 1;
					printf("Shift v[%d] << 1 and v[F] to MSB of v[%d]\n", x, x);	
					pc+=2;	
					break;
				
				default:
					printf("Unsupported opcode: %04x\n. System exit\n", opcode);
					exit(EXIT_FAILURE);	
					break;
			}
			break;

		case 0x9000://9XY0 - skip next instruction if v[x] != v[y]
			x = (opcode & 0xF00) >> 8;
			y = (opcode & 0x0F0) >> 4;

			if( v[x] != v[y] ){
				printf("Skipping next instruction since v[%d] != v[%d]\n", x, y);
				pc+=4;
			}else{
				printf("Not skipping next instruction since v[%d] == v[%d]\n", x, y);
				pc+=2;
			}			
			break;
	
		case 0xA000://ANNN - set i to NNN
			address = opcode & 0x0FFF;
			i = address;
			pc+=2;
			printf("Set i to %04x\n", i);
			break;

		case 0xB000://BNNN - jump to address NNN + v[0]
			address = (opcode & 0x0FFF) + (v[0] & 0xFF);
			pc=address;
			printf("Set pc to the address %04x\n", address);
			break;

		case 0xC000://CXNN - set v[X] to a random number and NN
			x = (opcode & 0x0F00) >> 8;
			y = (opcode & 0x00FF);//NN
			_x = rand() % 256;//random number betwwen 0 and 255;	
			v[x] = _x & y;	
			pc+=2;	
			printf("v[%d] has been set to (rand)%d\n", x, v[x]);	
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
	
		case 0xE000:
			switch(opcode & 0x00FF){
				case 0x009E://EX9E - skip next instr if the key v[X] is pressed
					x = (opcode & 0x0F00) >> 8;
					if(keys[ v[x] ] == 1)
						pc+=4;
					else
						pc+=2;
					break;
				case 0x00A1://EXA1 - skip next instr if the key v[X] is not pressed
					x = (opcode & 0x0F00) >> 8;
					if(keys[ v[x] ] == 0)
						pc+=4;
					else
						pc+=2;
					printf("Skipping next instruction if v[%d] is NOT pressed\n", x);
					break;

				default:
					printf("Unsupported opcode: %04x. System exit\n", opcode);
					exit(EXIT_FAILURE);
					break;
			}	
			break;		

		case 0xF000://multicase
			switch(opcode & 0x00FF){

				case 0x0007://FX07 - set v[X] to the value of delay_timer
					x = (opcode & 0x0F00) >> 8;
					v[x] = delay_timer;
					pc+=2;
					printf("v[%d] has been set to %d\n", x, delay_timer);
					break;

				case 0x000A://FX0A - A key press is awaited, and then stored in v[x]
					x = (opcode & 0x0F00) >> 8;
					waitKey = true;
					keylocation = x;	
					pc+=2;
					break;	
	
				case 0x0015://FX15 - set delay timer to v[X]
					x = (opcode & 0x0F00) >> 8;
					delay_timer = v[x];
					pc+=2;
					printf("Set delay timer to v[%d] = %d\n", x, delay_timer);	
					break;

				case 0x0018://FX18 - set the sound timer to v[x]
					x = (opcode & 0x0F00) >> 8;
					sound_timer = v[x];
					pc+=2;
					break;

				case 0x001E://FX1E - adds v[x] to i
					x = (opcode & 0x0F00) >> 8;
					i += v[x];
					printf("Adding v[%d] = %d to i\n", x, v[x]);
					pc+=2;		
					break;

				case 0x0029://FX29 - set i to location of the sprite for character vx (fontset)
					_x = v[ (opcode & 0x0F00) >> 8];
					i = 0x050 + _x * 5;
					printf("Setting i to character v[%d] = %d offset to %04x", 
						(opcode & 0x0F00) >> 8, _x, i);
					pc+=2;
				
					break;	
				case 0x0033://FX33 - store a binary coded decimal value VX in i,i+1,i+2
					_x = v[ (opcode & 0x0F00) >> 8];
					memory[i] = _x / 100;
					memory[i+1] = (_x%100)/10;
					memory[i+2] = _x % 10;

					pc+=2;
					printf("Storing binary coded decimal V[ %d ] = %d as {%d, %d, %d}\n", (opcode & 0x0F00) >> 8, _x, memory[i], memory[i+1], memory[i+2]); 

					break;
				
				case 0x0055://FX55 - stores v0 to vx in memory starting at adress i
					x = (opcode & 0xF00) >> 8;
					for(y = 0; y <= x; y++){
						memory[i + y] = v[i];
					}
					printf("Setting memory[%04x+n] = v[0] to v[%d]", i, x);
						
					pc+=2;
					break;
	
				case 0x0065://FX65 - fills v0 to vx with values from i
					x = (opcode & 0xF00) >> 8;
					for(y = 0; y <= x; y++){
						v[y] = memory[i+y];
					}

					printf("Setting v[0] to v[%d] to the values of memory[%04x]\n", x, i&0xFFFF);
					
					i += x + 1;
					pc+=2;	
					break;

				default:
					printf("Unsupported opcode: %04x. System exit\n", opcode);
					exit(EXIT_FAILURE);
					break;
			}
			break;
			
		default:
			printf("Unsupported opcode: %04x. System exit\n", opcode);
			exit(EXIT_FAILURE);	
			break;
	}	
	
	if(sound_timer > 0)
		sound_timer--;
	if(delay_timer > 0)
		delay_timer--;
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

void display_handle_input(){
	int key = display_handle_keys();
	printf("Just got the key %d\n", key);

	if(key == KEY_QUIT){
		running = false;
	}else if(key == KEY_CLEAR){
		memset(keys, 0, sizeof(keys));
	}

	if(key >= KEY_ONE && key <= KEY_V){
		keys[key] = 1;

		if(waitKey){
			v[keylocation] = key;
			waitKey = false;
		}
	}
}

int display_handle_keys(){
	const unsigned char * keystate;
	int i;
	
	SDL_Delay(1000 / 650);

	SDL_Event e;
	
	while(SDL_PollEvent(&e) != 0){
		if(e.type == SDL_QUIT)
			return KEY_QUIT;
		else if(e.type == SDL_KEYUP)
			return KEY_CLEAR;
	}

	keystate = SDL_GetKeyboardState(NULL);

	for(i = 0; i < KEY_SIZE; i++){
		if(keystate[key_key[i]])
			return key_value[i];
	}

	return KEY_NO;
}
