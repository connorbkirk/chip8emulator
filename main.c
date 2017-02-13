#include "chip.h"

void start(){
	while(1){
		chip_run();
		SDL_Event e;
		if(SDL_PollEvent(&e)){
			if(e.type == SDL_QUIT)
				exit(EXIT_SUCCESS);
		}
		if(needsRedraw){
			needsRedraw = false;
			display_update();
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

