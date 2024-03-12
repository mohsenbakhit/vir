//
// Created by Mohsen Bakhit on 2024-03-05.
// includes
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

// data

struct termios terminal_settn;

// terminal
void shutdown(const char* c) {
    perror(c);
    exit(1);
}

void DisableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settn);
}

void EnableRawMode() {
    tcgetattr(STDIN_FILENO, &terminal_settn);

    atexit(DisableRawMode);
    struct termios raw = terminal_settn;
    terminal_settn.c_iflag = ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                           | INLCR | IGNCR | ICRNL | IXON);
    terminal_settn.c_oflag &= ~OPOST;
    terminal_settn.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    terminal_settn.c_cflag &= ~(CSIZE | PARENB);
    terminal_settn.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// init

int main() {
    EnableRawMode();
    char input;
    while (read(STDIN_FILENO, &input, 1) == 1 && input != 'q') {
    }
    return 0;
}
