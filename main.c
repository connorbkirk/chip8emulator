#include "chip.h"

void start(){
	while(1){
		chip_run();
		if(needsRedraw){
			display_update();	
			removeDrawFlag();
		}
		sleep(10);
	}
}

int main(int argv, char ** argc){
	chip_init();
	chip_load("fuck");
	display_init();	
	start();
}
