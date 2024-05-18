#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editor {
    int screen_rows;
    int screen_cols;
    struct termios default_terminal_config;
};

struct editor editor;

void enable_raw_mode();
void disable_raw_mode();
void panic(const char *s);

char get_key_press();
void handle_key_press();

void setup_editor();
void refresh_screen();
int get_window_size(int *rows, int *cols);
int get_cursor_position(int *rows, int *cols);
void draw_lines();

int main() {

    enable_raw_mode();
    setup_editor();

    while(1) {

        refresh_screen();
        handle_key_press();
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

void handle_key_press() {

    char c = get_key_press();

    switch(c) {

        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

void enable_raw_mode() {

    int error_code = tcgetattr(STDIN_FILENO, &editor.default_terminal_config);
    
    if (error_code == -1) {
        panic("error getting attribute STDIN_FILENO");
    }

    atexit(disable_raw_mode);

    struct termios custom_terminal_config = editor.default_terminal_config;
    custom_terminal_config.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    custom_terminal_config.c_oflag &= ~(OPOST);
    custom_terminal_config.c_cflag |= (CS8);
    custom_terminal_config.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    custom_terminal_config.c_cc[VMIN] = 0;
    custom_terminal_config.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &custom_terminal_config);
}

void disable_raw_mode() {

    int error_code = tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.default_terminal_config);

    if (error_code == -1) {
        panic("error setting atrtibute STDIN_FILENO");
    }
}

void panic(const char *s) {

    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

void setup_editor() {

    int code = get_window_size(&editor.screen_rows, &editor.screen_cols);

    if (code == -1) {
        panic("could not get window size");
    }
}

void refresh_screen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    draw_lines();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

int get_window_size(int *rows, int *cols) {

    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return -1;
        }

        return get_cursor_position(rows, cols);
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

int get_cursor_position(int *rows, int *cols) {

    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        return -1;
    }

    while(i < sizeof(buf) - 1) {
        if (
            read(STDIN_FILENO, &buf[i], 1) != 1 ||
            buf[i] == 'R'
        ) {
            break;
        }
        i++;
    }

    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    }

    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    }

    return 0;
}

void draw_lines() {

    for (int row = 0; row < editor.screen_rows; row++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}