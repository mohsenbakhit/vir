//
// Created by Mohsen Bakhit on 2024-03-05.
//
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
struct trow {
    char* text;
};

struct termios terminal_settn;

void DisableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settn);
}

void EnableRawMode() {
    tcgetattr(STDIN_FILENO, &terminal_settn);

    atexit(DisableRawMode);
    struct termios raw = terminal_settn;

    terminal_settn.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    EnableRawMode();
    char input;
    while (read(STDIN_FILENO, &input, 1) == 1 && input != 'q');
    return 0;
}
