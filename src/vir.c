#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <ncurses.h>

int mode = 0; //0 is normal, 1 is insert
bool is_first_char = true;
void handle_unctrl(int ch) {
  if (ch == 10) {
    addch('\n');
  }
}
int main(){
  initscr();
  noecho();
  cbreak();
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
      if (unctrl(input)[0] == input)
        addch(input);
      else{
        handle_unctrl(input);
      }
    }
  }

  endwin();
  return 0;
}