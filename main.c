#include "chip.h"

void start(){
	running = true;	
	while(running){
		if(!waitKey)
			chip_run();
		display_handle_input();
		if(needsRedraw){
			needsRedraw = false;
			display_update();
		}	
		//usleep(20000);
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

