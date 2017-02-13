#include "chip.h"

void start(){
	while(1){
		chip_run();
		if(needsRedraw){
			needsRedraw = false;
			display_update();
		}	
		usleep(50000);
	}
}

int main(int argv, char ** argc){
	atexit(display_destroy);	

	chip_init();
	chip_load_file(argc[1]);
	display_init();	
	start();

	return EXIT_SUCCESS;
}

