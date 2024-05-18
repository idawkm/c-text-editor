#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define BUFFER_INIT {NULL, 0}
#define EDITOR_VERSION "1.0.0"

struct buffer {
    char *data;
    int length;
};

typedef struct erow {
    int size;
    char *text;
} erow;

struct editor {
    int x;
    int y;
    int screen_rows;
    int screen_cols;
    int num_rows;
    erow row;
    struct termios default_terminal_config;
};

struct editor editor;

enum arrows {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

void enable_raw_mode();
void disable_raw_mode();
void append_buffer(struct buffer *buffer, const char *s, int length);
void release_buffer(struct buffer *buffer);
void move_cursor(int key);
void panic(const char *s);

int get_key_press();
void handle_key_press();

void setup_editor();
void open_editor();
void refresh_screen();
int get_window_size(int *rows, int *cols);
int get_cursor_position(int *rows, int *cols);
void draw_header(struct buffer *buffer);
void draw_lines(struct buffer *buffer);

int main() {

    enable_raw_mode();
    setup_editor();
    open_editor();

    while(1) {

        refresh_screen();
        handle_key_press();
    }
    return 0;
}

void move_cursor(int key) {

    switch(key) {
        
        case ARROW_LEFT:

            if (editor.x != 0) {
                editor.x--;
            }
            break;
        case  ARROW_RIGHT:
            if (editor.x != editor.screen_cols - 1) {
                editor.x++;
            }
            break;
        case ARROW_UP:
            if (editor.y != 0) {
                editor.y--;
            }
            break;
        case ARROW_DOWN:

            if (editor.y != editor.screen_rows - 1) {
                editor.y++;
            }
            break;
    }
}

void append_buffer(struct buffer *buffer, const char *s, int length) {

    char *data = realloc(buffer->data, buffer->length + length);

    if (data == NULL) {
        return;
    }

    memcpy(&data[buffer->length], s, length);
    buffer->data = data;
    buffer->length += length;
}

void release_buffer(struct buffer *buffer) {
    free(buffer->data);
}

int get_key_press() {

    int nread;
    char c;

    while((nread = read(STDIN_FILENO, &c, 1)) != 1) {

        if (nread == -1 && errno != EAGAIN) {
            panic("could not read input from STDIN_FILENO");
        }
    }

    if (c == '\x1b') {

        char sequence[3];

        if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
            return '\x1b';
        }

        if (read(STDIN_FILENO, &sequence[1], 1) != 1) {
            return '\x1b';
        }

        if (sequence[0] == '[') {

            switch(sequence[1]) {

                case 'A': return ARROW_UP;
                case 'B': return ARROW_DOWN;
                case 'C': return ARROW_RIGHT;
                case 'D': return ARROW_LEFT;
            }
        } 

        return '\x1b';
    }

    return c;
}

void handle_key_press() {

    int c = get_key_press();

    switch(c) {

        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case ARROW_UP:
        case ARROW_DOWN:
            move_cursor(c);
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

    editor.num_rows = 0;
    editor.x = 0;
    editor.y = 0;
    int code = get_window_size(&editor.screen_rows, &editor.screen_cols);

    if (code == -1) {
        panic("could not get window size");
    }
}

void open_editor() {
    char *line = "Hello, World";
    ssize_t size = strlen(line);

    editor.row.size = size;
    editor.row.text = malloc(size + 1);
    memcpy(editor.row.text, line, size);
    editor.row.text[size] = '\0';
    editor.num_rows = 1;
}

void refresh_screen() {

    struct buffer buffer = BUFFER_INIT;

    append_buffer(&buffer, "\x1b[?25l", 6);
    append_buffer(&buffer, "\x1b[H", 3);

    draw_lines(&buffer);

    char cursor[32];
    snprintf(cursor, sizeof(cursor), "\x1b[%d;%dH", editor.y + 1, editor.x+1);
    append_buffer(&buffer, cursor, strlen(cursor));

    append_buffer(&buffer, "\x1b[?25h", 6);
    write(STDOUT_FILENO, buffer.data, buffer.length);
    release_buffer(&buffer);
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

void draw_header(struct buffer *buffer) {

    char message[40];
    int message_length = snprintf(message, sizeof(message), "My editor - version %s", EDITOR_VERSION);

    if (message_length > editor.screen_cols) {
        message_length = editor.screen_cols;
    }

    int padding = (editor.screen_cols - message_length) / 2;
    if (padding) {
        append_buffer(buffer, "~", 1);
        padding--;
    }

    while(padding--) {
        append_buffer(buffer, " ", 1);
    }

    append_buffer(buffer, message, message_length);
}

void draw_lines(struct buffer *buffer) {

    draw_header(buffer);

    for (int row = 1; row < editor.screen_rows; row++) {
        
        append_buffer(buffer, "~", 1);
        append_buffer(buffer, "\x1b[K", 3);

        if (editor.num_rows > 0 && row == 2) {
            int size = editor.row.size;
            if (size > editor.screen_cols) {
                size = editor.screen_cols;
            }

            append_buffer(buffer, editor.row.text, size);
        }
        
        if (row < editor.screen_rows - 1) {
            append_buffer(buffer, "\r\n", 2);
        }
    }
}