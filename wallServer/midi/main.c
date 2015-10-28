#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <termios.h>

#include "keyboard.h"
#include "main.h"
#include <portmidi.h>


static int running = 1;
static void leave(int sig) { running = 0; }


int main(int argc, char** argv) {
	signal(SIGINT, leave);

	keyboard_init();


	printf("%i\n",Pm_CountDevices());

	KeyboardEvent e;

	while(running) {

		while(keyboard_poll(&e)) {
			printf("%d %d %d\n", e.x, e.y,e.type);
			fflush(stdout);
		}
		usleep(1000);
	}

	keyboard_kill();

	return 0;
}


