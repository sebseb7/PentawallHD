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

int tty_fd;


uint8_t fac1 = 255;
uint8_t fac2 = 1;

// 48 50 52 53 55 57 59 60 62  64 65 67

static int running = 1;
static void leave(int sig) { running = 0; }


int main(int argc, char** argv) {
	signal(SIGINT, leave);


	keyboard_init();


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


