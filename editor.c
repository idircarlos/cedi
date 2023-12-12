#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "editor.h"
#include "common.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}

#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

void editorInit(Editor *e) {
    e->cx = 0;
    e->cy = 0;
    e->rowoff = 0;
    e->coloff = 0;
    e->nrows = 0;
    e->lines = NULL;
    if (getWindowSize(&e->screenrows,  &e->screencols) == -1) DIE("getWindowSize");
}

int editorReadKey(Editor *e) {
    (void) e;
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

void editorMoveCursor(Editor *e, int key) {
    switch (key) {
        case K_LEFT:
            if (e->cx > 0) e->cx--;
            break;
        case K_RIGHT:
            e->cx++;
            break;
        case K_UP:
            if (e->cy > 0) e->cy--;
            break;
        case K_DOWN:
            if (e->cy < e->nrows) e->cy++;
            break;
        }
}

void editorProcessKeypress(Editor *e) {
    int c = editorReadKey(e);
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
            write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
            exit(0);
            break;

        case K_HOME:
            e->cx = 0;
            break;
        case K_END:
            e->cx = e->screencols - 1;
            break;
        case K_PAGE_UP:
        case K_PAGE_DOWN: {
                int times = e->screenrows;
                while (times--) editorMoveCursor(e, c == K_PAGE_UP ? K_UP : K_DOWN);
            }
            break;
        case K_UP:
        case K_DOWN:
        case K_LEFT:
        case K_RIGHT:
            editorMoveCursor(e, c);
        default:
            break;
    }
}

void editorDrawRows(Editor *e, ABuf *ab) {
    int y;
    for (y = 0; y < e->screenrows; y++) {
        int fileline = y + e->rowoff;
        if (fileline >= e->nrows) {
            if (e->nrows == 0 && y == e->screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Cedi editor -- version %s", CEDI_VERSION);
                if (welcomelen > e->screencols) welcomelen = e->screencols;
                int padding = (e->screencols - welcomelen) / 2;
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
        }
        else {
            int len = e->lines[fileline].len - e->coloff;
            if (len < 0) len = 0;
            if (len > e->screencols) len = e->screencols;
            abAppend(ab, &e->lines[fileline].chars[e->coloff], len);
        }
        abAppend(ab, "\x1b[K", 3);
        if (y < e->screenrows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorRefreshScreen(Editor *e) {
    editorScroll(e);

    ABuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);  // Hides the cursor while repainting
    abAppend(&ab, "\x1b[H", 3);     // Reposition the cursor to top left
    
    editorDrawRows(e, &ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (e->cy - e->rowoff) + 1, (e->cx - e->coloff) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);  // Restores the visibility of the cursor
    
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorOpen(Editor *e, const char * filename) {
    FILE *fd = fopen(filename, "r");
    if (!fd) DIE("fopen");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fd)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
        editorAppendRow(e, line, linelen);
    }
    free(line);
    fclose(fd);
}

void editorAppendRow(Editor *e, char *s, size_t len) {
    e->lines = realloc(e->lines, sizeof(Line) * (e->nrows + 1));
    int at = e->nrows;
    e->lines[at].len = len;
    e->lines[at].chars = malloc(len + 1);
    memcpy(e->lines[at].chars, s, len);
    e->lines[at].chars[len] = '\0';
    e->nrows++;
}

void editorScroll(Editor *e) {
    if (e->cy < e->rowoff) {
        e->rowoff = e->cy;
    }
    if (e->cy >= e->rowoff + e->screenrows) {
        e->rowoff = e->cy - e->screenrows + 1;
    }
    if (e->cx < e->coloff) {
        e->coloff = e->cx;
    }
    if (e->cx >= e->coloff + e->screencols) {
        e->coloff = e->cx - e->screencols + 1;
    }
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