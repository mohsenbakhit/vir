//
// Created by Mohsen Bakhit on 2024-03-05.

/*** includes ***/
#include <ctype.h>
#include <curses.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** defines ***/

#define CTRL_KEY_MASK(k) ((k) & 0b00011111)
#define BUF_INIT                                                               \
  { NULL, 0 }

/*** data ***/
struct EditorConfig {
  int screen_rows;
  int screen_cols;
  struct termios orig_termios;
};

struct EditorConfig E;
; /*** terminal ***/

void shutdown(const char *c) {
  perror(c);
  exit(1);
}

void DisableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
    shutdown("tcsetattr");
  }
}

void EnableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    shutdown("tcgetattr");
  atexit(DisableRawMode);

  struct termios raw = E.orig_termios;
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
  ssize_t num_read;
  while ((num_read = read(STDIN_FILENO, &input, 1)) != 1) {
    if (num_read == -1 && errno != EAGAIN)
      shutdown("read");
  }
  return input;
}

int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    return -1;
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void initEditor() {
  if (getWindowSize(&E.screen_rows, &E.screen_cols) == -1)
    shutdown("getWindowSize");
}

/*** output ***/

struct buffer {
  char *buf;
  int len;
};

void bufferAppend(struct buffer *bf, const char *text, int len) {
  char *new_text = realloc(bf->buf, bf->len + len);

  if (new_text == NULL)
    return;
  memcpy(&new_text[bf->len], text, len);
  bf->buf = text;
  bf->len += len;
}

void bufferFree(struct buffer *bf) { free(bf->buf); }

void EditorDrawRows(struct buffer *buf) {
  for (int t = 0; t < E.screen_rows; t++) {
    bufferAppend(buf, "~", 1);

    bufferAppend(buf, "\x1b[K", 3);
    if (t < E.screen_rows - 1) {
      bufferAppend(buf, "\r\n", 2);
    }
  }
}
void EditorRefreshScreen() {
  struct buffer buf = BUF_INIT;

  bufferAppend(&buf, "\x1b[?25l", 6);
  bufferAppend(&buf, "\x1b[H", 3);

  EditorDrawRows(&buf);
  bufferAppend(&buf, "\x1b[H", 3);
  bufferAppend(&buf, "\x1b[?25l", 6);

  write(STDOUT_FILENO, buf.buf, buf.len);
  bufferFree(&buf);
}

/*** input ***/

void ProcessKeyPress() {
  char input = EditorKeyRead();

  // Process specific keys and combinations

  switch (input) {
  case CTRL_KEY_MASK('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
  write(STDOUT_FILENO, &input, 1);
}

/*** init ***/

int main() {
  EnableRawMode();
  initEditor();
  while (1) {
    EditorRefreshScreen();
    ProcessKeyPress();
  }
  return 0;
}
