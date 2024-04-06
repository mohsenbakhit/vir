#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ncurses.h>

int mode = 0; //0 is normal, 1 is insert
int main(){
  initscr();
  noecho();
  cbreak();

  while(true) {
    char input = getch();

    if (input == 'i') {
      mode = 1;
    } else if (input == 27 && mode == 1) {
      mode = 0;
    } else if (mode == 1) {
      addch(input);
    }
  }

  endwin();
  return 0;
}