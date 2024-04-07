#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ncurses.h>

#define BACKSPACE_KEY 127
#define ENTER_KEY 10
int mode = 0; // 0 is normal, 1 is insert
bool is_first_char = true;
int y, x;

void handle_unctrl(int ch) {
  if (ch == ENTER_KEY) {
    addch('\n');
  }
  if (ch == BACKSPACE_KEY) {
    getyx(stdscr, y, x);
    if (x > 0) {
      move(y, x - 1);
      addch(' ');
    }
  }
}

int main(){
  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  
  getyx(stdscr, y, x);

  addstr("~         VIR: VI Regressed          ~");

  while(true) {
    int input = getch();
    if (input == 'i' && mode == 0) {
      mode = 1;
      if (is_first_char) {
        clear();
        is_first_char = false;
      }
    } else if (input == 27 && mode == 1) {
      mode = 0;
    } else if (mode == 1) {
      if (unctrl(input)[0] == input) {
        addch(input);
      }
      else{
        handle_unctrl(input);
      }
    }
  }

  endwin();
  return 0;
}