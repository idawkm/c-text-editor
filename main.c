#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)
struct termios default_terminal_config;

void enable_raw_mode();
void disable_raw_mode();
void panic(const char *s);

char get_key_press();
void on_key_press();

int main() {

    enable_raw_mode();
    
    while(1) {

        on_key_press();
    }
    return 0;
}

char get_key_press() {

    int nread;
    char c;

    while((nread = read(STDIN_FILENO, &c, 1)) != 1) {

        if (nread == -1 && errno != EAGAIN) {
            panic("could not read input from STDIN_FILENO");
        }
    }

    return c;
}

void on_key_press() {

    char c = get_key_press();

    switch(c) {

        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

void enable_raw_mode() {

    int error_code = tcgetattr(STDIN_FILENO, &default_terminal_config);
    
    if (error_code == -1) {
        panic("error getting attribute STDIN_FILENO");
    }

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

    int error_code = tcsetattr(STDIN_FILENO, TCSAFLUSH, &default_terminal_config);

    if (error_code == -1) {
        panic("error setting atrtibute STDIN_FILENO");
    }
}

void panic(const char *s) {

    perror(s);
    exit(1);
}