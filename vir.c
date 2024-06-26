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
uint8_t mode = NORMAL_MODE;
// terminal
void die(const char *s) {
	perror(s);
	exit(EXIT_FAILURE);
}

void disable_raw_mode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
		die("tcsetattr");
}

void enable_raw_mode(void) {
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

char editor_key_read(void) {
	int bytes_read;
	char c;
	while ((bytes_read = read(STDIN_FILENO, &c, 1) == -1) && errno != EAGAIN) {
		if (bytes_read == -1 && errno != EAGAIN) die("read");
	}
	return c;
}

void editor_process_key_press(void) {
	char c = editor_key_read();

	switch(c) {
		if (mode == NORMAL_MODE) {
			if (c == 'i') mode = INSERT_MODE;
			if (c == CTRL_KEY('x')) {
				exit(EXIT_SUCCESS);
			}
		} else if (mode == INSERT_MODE) {
			if (c == ESC) {
				mode = NORMAL_MODE;
				break;
			}
			if (iscntrl(c)) {
				printf("%d \r\n", c);
			} else {
				printf("%d ('%c')", c, c);
			}
		}
	}
}
// main

int main(void){
	enable_raw_mode();
	
	while (1) {
		editor_process_key_press();
	}
	return 0;
}
