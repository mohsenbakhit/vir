// includes
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>

// defines
#define NORMAL_MODE 0
#define COMMAND_MODE 2
#define INSERT_MODE 1
#define ESC 27
#define CTRL_KEY(k) ((k) & 0x1f)
// data structures
struct termios orig_termios;

// terminal
void die(const char *s) {
	perror(s);
	exit(EXIT_FAILURE);
}

void disable_raw_mode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enable_raw_mode() {
	if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disable_raw_mode);

	struct termios raw = orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

// main

int main(){
	enable_raw_mode();

	char c;
	uint8_t mode = NORMAL_MODE;
	while (1) {
		c = '\0';
		if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
		if (mode == INSERT_MODE) {
			if (c == ESC) mode = NORMAL_MODE;
			if (iscntrl(c)) {
				printf("%d \r\n", c);
			} else {
				printf("%d ('%c')", c, c);
			}
		}
		if (c == 'i' && mode == NORMAL_MODE) {
		
			mode = INSERT_MODE;
		}
		if (c == CTRL_KEY('q') && mode == NORMAL_MODE){
			break;
		}
	}
	return 0;
}
