#include "chip.h"

void signal_handler(int sig){
	SDL_Quit();	
	abort();
}

void start(){
	while(1){
		if(event.type == SDL_QUIT)
			exit(0);
		if(SDL_PollEvent(&event))
			continue;
		chip_run();
		if(needsRedraw){
			display_update();	
			removeDrawFlag();
		}
		usleep(10);
	}
}

int main(int argv, char ** argc){
	signal(SIGINT, signal_handler);	

	chip_init();
	chip_load(argc[1]);
	display_init();	
	start();

	atexit(SDL_Quit);	
	return EXIT_SUCCESS;
}

