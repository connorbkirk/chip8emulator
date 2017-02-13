#include "chip.h"

void start(){
	while(1){
		SDL_Event e;
		if(e.type == SDL_QUIT)
			exit(0);
		if(SDL_PollEvent(&e))
			continue;
		chip_run();
		if(needsRedraw){
			display_update();	
			removeDrawFlag();
		}
		usleep(10000);
	}
}

int main(int argv, char ** argc){
	chip_init();
	chip_load_file(argc[1]);
	display_init();	
	start();

	atexit(SDL_Quit);	
	return EXIT_SUCCESS;
}

