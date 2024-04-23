#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <ncurses.h>

#define BUFFER_SIZE 1024
#define BACKSPACE_KEY 127
#define ENTER_KEY 10
#define ESCAPE_KEY 27

int mode = 0; // 0 is normal, 1 is insert
bool is_first_char = true;
int y, x;
FILE *file;

typedef struct {
  char* data;
  int capacity;
  int length;
} Buffer;

Buffer* create_buffer() {
  Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
  if (buffer == NULL) {
    fprintf(stderr, "Memory allocation failed\n"); 
    exit(1);
  }

  buffer->data = (char*)malloc(BUFFER_SIZE * sizeof(char));
  if (buffer->data == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
     exit(1);
  }
  buffer->capacity = BUFFER_SIZE;
  buffer->length = 0;
  return buffer;
}

void free_buffer(Buffer* buffer) {
  free(buffer->data);
  free(buffer);  
}

void write_buffer(Buffer* buffer) {
  fwrite(buffer->data, sizeof(char), buffer->length, file);
  buffer->data = "";
  buffer->length = 0;
}

void insert_text(Buffer *buffer, const char* text) {
  int text_length = strlen(text);
  if (buffer->length + text_length >= buffer->capacity) {
    write_buffer(buffer);
  }
  strcpy(buffer->data + buffer->length, text);
  buffer->length += text_length;
}

void handle_unctrl(Buffer *buffer, int ch) {
  if (ch == ENTER_KEY) {
    addch('\n');
    insert_text(buffer, "\n");
  }
  if (ch == BACKSPACE_KEY) {
    getyx(stdscr, y, x);
    if (x > 0) {
      move(y, x - 1);
      addch(' ');
    }
  }
}

const char* getch_to_const_char(int ch) {
    static char buffer[2];

    buffer[0] = ch;
    buffer[1] = '\0';

    return buffer;
}

int main(int argc, char *argv[]){
  if (argc != 2) {
    printf("Usage: ./vir <filename>");
    return 1;
  }

  file = fopen(argv[1], "w+");
  if (file == NULL) {
    printf("Error creating %s", argv[1]);
    fclose(file);
    return 1;
  }

  Buffer *buffer = create_buffer();

  initscr();
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  
  getyx(stdscr, y, x);

  addstr("~         VIR: VI Regressed          ~\n");
  addstr("~ i: insert mode | w: write to file | q: quit");
  while(true) {
    int input = getch();
    
    if (mode == 0) {
      if (input == 'i') {
        mode = 1;
        if (is_first_char) {
          clear();
          is_first_char = false;
          }
      }
      if (input == 'w') {
        write_buffer(buffer);
      }
      if (input == 'q') {
        return 1;
      }
    } else if (input == ESCAPE_KEY && mode == 1) {
      mode = 0;
    } else if (mode == 1) {
      if (unctrl(input)[0] == input) {
        addch(input);

        insert_text(buffer, getch_to_const_char(input));
      }
      else{
        handle_unctrl(buffer, input);
      }
    }
  }
  fclose(file);
  endwin();
  return 0;
}