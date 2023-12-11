#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

// Macros
#define CTRL_KEY(k) ((k) & 0x1f)
#define DIE(s) die(__FILE__, __LINE__, s)
#define ABUF_INIT {NULL, 0}
#define CEDI_VERSION "0.0.1"

typedef struct {
    char *b;
    int len;
} ABuf;

// Prototypes
void die(char *file, int line, char *s);
void disableRawMode();
void enableRawMode();
int editorReadKey();
void editorMoveCursor(int key);
void editorProcessKeypress();
void editorDrawRows(ABuf *ab);
void editorRefreshScreen();
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
void abAppend(ABuf *ab, const char *s, int len);
void abFree(ABuf *ab);
void initEditor();

typedef enum {
    K_LEFT = 1000,
    K_RIGHT,
    K_UP,
    K_DOWN,
    K_HOME,
    K_END,
    K_PAGE_UP,
    K_PAGE_DOWN,
    K_DEL
} EditorKey;

typedef struct {
    int cx;
    int cy;
    int screenrows;
    int screencols;
    struct termios orig_termios;
} Editor;

Editor E;

void die(char *file, int line, char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
    write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
    fprintf(stderr, "%s:%d %s:%s\n", file, line, s, strerror(errno));
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) DIE("tcsetattr");
}
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) DIE("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = E.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) DIE("tcsetattr");
}

int editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) DIE("read");
    }
    if (c == '\x1b') {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return K_HOME;
                        case '3': return K_DEL;
                        case '4': return K_END;
                        case '5': return K_PAGE_UP;
                        case '6': return K_PAGE_DOWN;
                        case '7': return K_HOME;
                        case '8': return K_END;
                    }
                }
            } 
            else {
                switch (seq[1]) {
                    case 'A': return K_UP;
                    case 'B': return K_DOWN;
                    case 'C': return K_RIGHT;
                    case 'D': return K_LEFT;
                    case 'H': return K_HOME;
                    case 'F': return K_END;
                }
            }
        }
        else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return K_HOME;
                case 'F': return K_END;
            }
        }
        return '\x1b';
    }
    return c;
}

int getCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
    return 0;
}

int getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    // If ioctl() fails, we will try to query the size 'by hand'
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
        return -1;
    }
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

void editorMoveCursor(int key) {
    switch (key) {
        case K_LEFT:
            if (E.cx > 0) E.cx--;
            break;
        case K_RIGHT:
            if (E.cx < E.screencols - 1) E.cx++;
            break;
        case K_UP:
            if (E.cy > 0) E.cy--;
            break;
        case K_DOWN:
            if (E.cy < E.screenrows - 1) E.cy++;
            break;
        }
}

void editorProcessKeypress() {
    int c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
            write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
            exit(0);
            break;

        case K_HOME:
            E.cx = 0;
            break;
        case K_END:
            E.cx = E.screencols - 1;
            break;
        case K_PAGE_UP:
        case K_PAGE_DOWN: {
                int times = E.screenrows;
                while (times--) editorMoveCursor(c == K_PAGE_UP ? K_UP : K_DOWN);
            }
            break;
        case K_UP:
        case K_DOWN:
        case K_LEFT:
        case K_RIGHT:
            editorMoveCursor(c);
        default:
            break;
    }
}

void editorDrawRows(ABuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        if (y == E.screenrows / 3) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "Cedi editor -- version %s", CEDI_VERSION);
            if (welcomelen > E.screencols) welcomelen = E.screencols;
            int padding = (E.screencols - welcomelen) / 2;
            if (padding) {
                abAppend(ab, "~", 1);
                padding--;
            }
            while (padding--) abAppend(ab, " ", 1);
            abAppend(ab, welcome, welcomelen);
        }
        else {
            abAppend(ab, "~", 1);
        }
        abAppend(ab, "\x1b[K", 3);
        if (y < E.screenrows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorRefreshScreen() {
    ABuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);  // Hides the cursor while repainting
    abAppend(&ab, "\x1b[H", 3);     // Reposition the cursor to top left
    
    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);  // Restores the visibility of the cursor
    
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void abAppend(ABuf *ab, const char *s, int len) {
    char *new = (char*)realloc(ab->b, ab->len + len);
    if (new == NULL) DIE("abAppend - realloc");
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(ABuf *ab) {
    free(ab->b);
}

void initEditor() {
    E.cx = 0;
    E.cy = 0;
    if (getWindowSize(&E.screenrows,  &E.screencols) == -1) DIE("getWindowSize");
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}