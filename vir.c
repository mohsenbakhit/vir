// includes
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <ctype.h>

// defines
#define NORMAL_MODE 0
#define COMMAND_MODE 2
#define INSERT_MODE 1
#define ESC 27
#define CTRL_KEY(k) ((k) & 0x1f)
#define VIR_V "0.0.1"
// data structures
struct editor_config {
	int screenCols;
	int screenRows;
	struct termios orig_termios;
};

struct editor_config E;
uint8_t mode = NORMAL_MODE;

// terminal
void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);
	
	
	perror(s);
	exit(EXIT_FAILURE);
}

void disable_raw_mode(void) {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void enable_raw_mode(void) {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	atexit(disable_raw_mode);

	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

//output

struct abuf {
	char *buf;
	int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
	char *new = realloc(ab->buf, ab->len + len);

	if (new == NULL) return;
	memcpy(&new[ab->len], s, len);
	ab->buf = new;
	ab->len += len;
}

void abFree(struct abuf *ab) {
	free(ab->buf);
}

void editor_draw_rows(struct abuf *ab) {
	int y;
	for (y = 0; y < E.screenRows; y++) {
		if (y == E.screenRows / 3) {
			char welcome[80];
			int welcomeLen = snprintf(welcome, sizeof(welcome),
			"VIR: Vi Regressed -- version %s", VIR_V);

			if (welcomeLen > E.screenCols) welcomeLen = E.screenCols;
			abAppend(ab, welcome, welcomeLen);
		} else {
			abAppend(ab, "~", 1);
		}

		abAppend(ab, "\x1b[K", 3);
		if (y < E.screenRows - 1) {
			abAppend(ab, "\r\n", 2);
		}
	}
}

void editor_refresh_screen() {
	struct abuf ab = ABUF_INIT;

	abAppend(&ab, "\x1b[?25l", 6);
	abAppend(&ab, "\x1b[H", 3);

	editor_draw_rows(&ab);

	abAppend(&ab, "\x1b[H", 3);
	abAppend(&ab, "\x1b[?25h", 6);

	write(STDOUT_FILENO, ab.buf, ab.len);
	abFree(&ab);
}

// input

char editor_key_read() {
	int num_read;
	char c;
	while ((num_read = read(STDIN_FILENO, &c, 1)) != 1) {
		if (num_read == -1 && errno != EAGAIN) die("read");
	}

	return c;
}

int get_cursor_position(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

	printf("\r\n");
	char c;

	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';

	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
	return 0;
}

int get_window_size(int *rows, int *cols) {
	struct winsize win_s;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win_s) == -1 || win_s.ws_col == 0) {
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
		return get_cursor_position(rows, cols);
	} else {
		*cols = win_s.ws_col;
		*rows = win_s.ws_row;

		return 0;
	}
}

void editor_process_key() {
	char c = editor_key_read();

	switch (mode) {
		case(NORMAL_MODE):
			switch (c) {
				case CTRL_KEY('q'):
					write(STDOUT_FILENO, "\x1b[2J", 4);
					write(STDOUT_FILENO, "\x1b[H", 3);
					exit(0);
					break;
				case CTRL_KEY('i'):
					mode = INSERT_MODE;
					break;
			}
			break;
		case (INSERT_MODE):
			switch(c) {
				case 27:
					mode = NORMAL_MODE;
					break;
				default:
					printf("%c", c);
					write(STDOUT_FILENO, &c, 1);
					break;
			}
			break;
		case(COMMAND_MODE):
			break;
	}

}

void init_editor() {
	if (get_window_size(&E.screenRows, &E.screenCols) == -1) die("get_window_size");
}

// main

int main() {
  enable_raw_mode();
  init_editor();
  while (1) {
	editor_refresh_screen();
    editor_process_key();
  }
  return 0;
}
