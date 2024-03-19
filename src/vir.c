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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settn) == -1) {
        shutdown("tcsetattr");
    }
}

void EnableRawMode() {
   if (tcgetattr(STDIN_FILENO, &terminal_settn) == -1) shutdown("tcgetattr");
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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) shutdown("tcsetattr");
}

// init

int main() {
    EnableRawMode();

    while (1) {
        char input = '\0';
        if (read(STDIN_FILENO, &input, 1) == -1) shutdown("read");
        if (iscntrl(input)) {
            printf("%d\r\n", input);
        } else {
            printf("%d ('%c')\r\n", input, input);
        }
        if (input == 'q') break;
    }
    return 0;
}
