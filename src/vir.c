//
// Created by Mohsen Bakhit on 2024-03-05.
//

struct trow {
    char* text;
};

#include <unistd.h>
int main() {
    char input;
    while (read(STDIN_FILENO, &input, 1) == 1 && input != 'q');
    return 0;
}
