#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

struct termios default_terminal_config;

void enable_raw_mode();
void disable_raw_mode();

int main() {

    enable_raw_mode();
    char c;
    while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {

        if (iscntrl(c)) {
            printf("%d\r\n", c);
        }
        else {
            printf("%c\r\n", c);
        }
    }
    return 0;
}

void enable_raw_mode() {

    tcgetattr(STDIN_FILENO, &default_terminal_config);
    atexit(disable_raw_mode);

    struct termios custom_terminal_config = default_terminal_config;
    custom_terminal_config.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    custom_terminal_config.c_oflag &= ~(OPOST);
    custom_terminal_config.c_cflag |= (CS8);
    custom_terminal_config.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    custom_terminal_config.c_cc[VMIN] = 0;
    custom_terminal_config.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &custom_terminal_config);
}

void disable_raw_mode() {

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &default_terminal_config);
}