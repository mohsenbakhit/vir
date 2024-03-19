//
// Created by Mohsen Bakhit on 2024-03-05.

/*** includes ***/
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY_MASK(k) ((k) & 0b00011111)

/*** data ***/

struct termios orig_termios;

// struct editorConfig E;
/*** terminal ***/
void shutdown(const char *c) {
  perror(c);
  exit(1);
}

void DisableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    shutdown("tcsetattr");
  }
}

void EnableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    shutdown("tcgetattr");
  atexit(DisableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    shutdown("tcsetattr");
}

char EditorKeyRead() {
  char input;
  int num_read;
  while ((num_read = read(STDIN_FILENO, &input, 1)) != 1) {
    if (num_read == -1 && errno != EAGAIN)
      shutdown("read");
  }
  return input;
}

/*** input ***/

void ProcessKeyPress() {
  char c = EditorKeyRead();

  // Process specific keys and combinations

  switch (c) {
  case CTRL_KEY_MASK('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}

/*** output ***/

void EditorDrawRows() {
  for (int t = 0; t < 24; t++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}
void EditorRefreshScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  EditorDrawRows();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** init ***/

int main() {
  EnableRawMode();

  while (1) {
    EditorRefreshScreen();
    ProcessKeyPress();
  }
  return 0;
}
