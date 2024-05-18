#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>
#include <termios.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define BUF_INIT {NULL, 0}

struct EditorConfig {
    int screen_rows;
    int screen_cols;
    struct termios orig_termios;
};
struct EditorConfig ec;

struct Buffer {
    char *b;
    int length;
};

void init_editor();
void draw_rows();
void enable_raw_mode();
void disable_raw_mode();
char editor_read_key();
void editor_refresh_screen();
void on_key_press();
void die(const char *s);
int get_window_size(int *rows, int *cols);
void buffer_append(struct Buffer *buf, const char *s, int length);
void buffer_free(struct Buffer *buf);

int main() {

    enable_raw_mode();
    init_editor();
    
    while(1) {
        editor_refresh_screen();
        on_key_press();
    };
    return 0;
}

void buffer_append(struct Buffer *buf, const char *s, int length) {

    char *new = realloc(buf->b, buf->length + length);

    if (new == NULL) {
        return;
    }

    memcpy(&new[buf->length], s, length);
    buf->b = new;
    buf->length += length;
}

void buffer_free(struct Buffer *buf) {
    free(buf->b);
}

void init_editor() {
    if (get_window_size(&ec.screen_rows, &ec.screen_cols) == -1) {
        die("get window size");
    }
}

void draw_rows(struct Buffer *buf) {

    for (int y = 0; y < ec.screen_rows; y++) {
        buffer_append(buf, "~", 1);

        if (y < ec.screen_rows - 1) {
            buffer_append(buf, "\r\n", 2);
        } 
    }
}

void editor_refresh_screen() {

    struct Buffer buf = BUF_INIT;

    buffer_append(&buf, "\x1b[2J", 4);
    buffer_append(&buf, "\x1b[2J", 3);
    
    draw_rows(&buf);

    write(STDOUT_FILENO, "\x1b[H", 3);
    buffer_free(&buf);
}

char editor_read_key() {
  
  int nread;
  char c;

  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
  }

  return c;
}

void on_key_press() {

    char c = editor_read_key();

    switch(c) {
        case CTRL_KEY('q'):
            editor_refresh_screen();
            exit(0);
            break;
    }
}

void enable_raw_mode() {

    tcgetattr(STDIN_FILENO, &ec.orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = ec.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8); 
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");
    }
}

void disable_raw_mode() {

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ec.orig_termios) == -1) {
        die("tcsetattr");
    }
}

void die(const char *s) {

    editor_refresh_screen();

    perror(s);
    exit(1);
}

int get_window_size(int *rows, int *cols) {

    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}