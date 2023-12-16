#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>

#include "editor.h"
#include "common.h"

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT {NULL, 0}
#define CEDI_QUIT_TIMES 3


void editorInit(Editor *e) {
    e->cx = 0;
    e->cy = 0;
    e->rx = 0;
    e->rowoff = 0;
    e->coloff = 0;
    e->nrows = 0;
    e->lines = NULL;
    e->dirty = 0;
    e->filename = NULL;
    e->statusmsg[0] = '\0';
    e->statusmsg_time = 0;
    if (getWindowSize(&e->screenrows,  &e->screencols) == -1) DIE("getWindowSize");
    e->screenrows -= 2; // Reserve two lines for the status bars
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
    Line *line = (e->cy >= e->nrows) ? NULL : &e->lines[e->cy];
    switch (key) {
        case K_LEFT:
            if (e->cx > 0) e->cx--;
            else if (e->cy > 0) {
                e->cy--;
                e->cx = e->lines[e->cy].len;
            }
            break;
        case K_RIGHT:
            if (line && e->cx < line->len) e->cx++;
            else if (line && e->cx == line->len) {
                e->cy++;
                e->cx = 0;
            }
            break;
        case K_UP:
            if (e->cy != 0) e->cy--;
            break;
        case K_DOWN:
            if (e->cy < e->nrows) e->cy++;
            break;
    }

    line = (e->cy >= e->nrows) ? NULL : &e->lines[e->cy];
    int linelen = line ? line->len : 0;
    if (e->cx > linelen) e->cx = linelen;
}

void editorProcessKeypress(Editor *e) {
    editorSetStatusMessage(e, CEDI_DEFAULT_STATUS_MESSAGE); // Reset status
    static int quit_times = CEDI_QUIT_TIMES;
    int c = editorReadKey(e);
    switch (c) {
        case CTRL_KEY('q'):
            if (e->dirty && quit_times > 0) {
                editorSetStatusMessage(e, "WARNING!!! File has unsaved changes. Press Ctrl-Q %d more times to quit.", quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
            write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
            exit(0);
            break;

        case CTRL_KEY('s'):
            editorSave(e);
            break;
        
        case '\r':
            editorInsertNewline(e);
            break;

        case K_HOME:
            e->cx = 0;
            break;
        case K_END:
            if (e->cy < e->nrows) e->cx = e->lines[e->cy].len;
            break;
        
        case K_BACKSPACE:
        case CTRL_KEY('h'):
        case K_DEL:
            if (c == K_DEL) editorMoveCursor(e, K_RIGHT);
            editorDelChar(e);
            break;
        case K_PAGE_UP:
        case K_PAGE_DOWN:
            {
                if (c == K_PAGE_UP) {
                    e->cy = e->rowoff;
                } 
                else if (c == K_PAGE_DOWN) {
                    e->cy = e->rowoff + e->screenrows - 1;
                    if (e->cy > e->nrows) e->cy = e->nrows;
                }
                int times = e->screenrows;
                while (times--) editorMoveCursor(e, c == K_PAGE_UP ? K_UP : K_DOWN);
            }
            break;
        case K_UP:
        case K_DOWN:
        case K_LEFT:
        case K_RIGHT:
            editorMoveCursor(e, c);
            break;
        case CTRL_KEY('l'):
        case '\x1b':
            break;
        default:
            editorInsertChar(e, c);
            break;
    }
    quit_times = CEDI_QUIT_TIMES;
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
            int len = e->lines[fileline].rlen - e->coloff;
            if (len < 0) len = 0;
            if (len > e->screencols) len = e->screencols;
            abAppend(ab, &e->lines[fileline].render[e->coloff], len);
        }
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(Editor *e, ABuf *ab) {
    abAppend(ab, "\x1b[7m", 4);     // Invert term colors
    char status[80];
    char rstatus[80];
    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s", e->filename ? e->filename : "[No Name]", e->nrows, e->dirty ? "(modified)" : "");
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d", e->cy + 1, e->nrows);
    if (len > e->screencols) len = e->screencols;
    abAppend(ab, status, len);
    while (len < e->screencols) {
        if (e->screencols - len == rlen) {            
            abAppend(ab, rstatus, rlen);
            break;
        }
        else {
            abAppend(ab, " ", 1);
            len++;
        }
    }
    abAppend(ab, "\x1b[m", 3);      // Revert colors to normal state
    abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(Editor *e, ABuf *ab) {
    abAppend(ab, "\x1b[K", 3);
    int msglen = strlen(e->statusmsg);
    if (msglen > e->screencols) msglen = e->screencols;
    if (msglen && time(NULL) - e->statusmsg_time < 5)
    abAppend(ab, e->statusmsg, msglen);
}

void editorRefreshScreen(Editor *e) {
    editorScroll(e);

    ABuf ab = ABUF_INIT;
    abAppend(&ab, "\x1b[?25l", 6);  // Hides the cursor while repainting
    abAppend(&ab, "\x1b[H", 3);     // Reposition the cursor to top left
    
    editorDrawRows(e, &ab);
    editorDrawStatusBar(e, &ab);
    editorDrawMessageBar(e, &ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (e->cy - e->rowoff) + 1, (e->rx - e->coloff) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);  // Restores the visibility of the cursor
    
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorSetStatusMessage(Editor *e, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(e->statusmsg, sizeof(e->statusmsg), fmt, ap);
    va_end(ap);
    e->statusmsg_time = time(NULL);
}

char *editorRowsToString(Editor *e, int *buflen) {
    int totlen = 0;
    int j;
    for (j = 0; j < e->nrows; j++)
        totlen += e->lines[j].len + 1;
    *buflen = totlen;
    char *buf = malloc(totlen);
    char *p = buf;
    for (j = 0; j < e->nrows; j++) {
        memcpy(p, e->lines[j].chars, e->lines[j].len);
        p += e->lines[j].len;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorOpen(Editor *e, const char *filename) {
    free(e->filename);
    e->filename = strdup(filename);
    FILE *fd = fopen(filename, "r");
    if (!fd) DIE("fopen");
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &linecap, fd)) != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r')) linelen--;
        editorInsertRow(e, e->nrows, line, linelen);
    }
    free(line);
    fclose(fd);
    e->dirty = 0;
}

void editorSave(Editor *e) {
    if (e->filename == NULL) {
        e->filename = editorPrompt(e, "Save as: %s (ESC to cancel)");
        if (e->filename == NULL) {
            editorSetStatusMessage(e, "Save aborted");
            return;
        }
    }
    int len;
    char *buf = editorRowsToString(e, &len);
    int fd = open(e->filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, len) != -1) {
            if (write(fd, buf, len) == len) {
                close(fd);
                free(buf);
                e->dirty = 0;
                editorSetStatusMessage(e, "%d bytes written to disk", len);
                return;
            }
        }
        close(fd);
    }
    free(buf);
    editorSetStatusMessage(e, "Can't save! I/O error: %s", strerror(errno));
}

void editorInsertRow(Editor *e, int at, char *s, size_t len) {
    if (at < 0 || at > e->nrows) return;
    e->lines = realloc(e->lines, sizeof(Line) * (e->nrows + 1));
    memmove(&e->lines[at + 1], &e->lines[at], sizeof(Line) * (e->nrows - at));
    
    e->lines[at].len = len;
    e->lines[at].chars = malloc(len + 1);
    memcpy(e->lines[at].chars, s, len);
    e->lines[at].chars[len] = '\0';
    

    e->lines[at].rlen = 0;
    e->lines[at].render = NULL;
    editorUpdateRow(e, &e->lines[at]);

    e->nrows++;
    e->dirty++;
}

void editorFreeRow(Editor *e, Line *line) {
    (void) e;
    free(line->chars);
    free(line->render);
}

void editorDelRow(Editor *e, int at) {
    if (at < 0 || at >= e->nrows) return;
    editorFreeRow(e, &e->lines[at]);
    memmove(&e->lines[at], &e->lines[at + 1], sizeof(Line) * (e->nrows - at - 1));
    e->nrows--;
    e->dirty++;
}

void editorUpdateRow(Editor *e, Line *line) {
    (void) e;
    int tabs = 0;
    int j;
    for (j = 0; j < line->len; j++)
        if (line->chars[j] == '\t') tabs++;

    free(line->render);
    line->render = malloc(line->len + tabs*(CEDI_TAB_STOP - 1) + 1);
    int idx = 0;
    for (j = 0; j < line->len; j++) {
        if (line->chars[j] == '\t') {
            line->render[idx++] = ' ';
            while (idx % CEDI_TAB_STOP != 0) line->render[idx++] = ' ';
        } else {
            line->render[idx++] = line->chars[j];
        }
    }
    line->render[idx] = '\0';
    line->rlen = idx;
}

void editorInsertNewline(Editor *e) {
    if (e->cx == 0) {
        editorInsertRow(e, e->cy, "", 0);
    } 
    else {
        Line *line = &e->lines[e->cy];
        editorInsertRow(e, e->cy + 1, &line->chars[e->cx], line->len - e->cx);
        line = &e->lines[e->cy];
        line->len = e->cx;
        line->chars[line->len] = '\0';
        editorUpdateRow(e, line);
    }
    e->cy++;
    e->cx = 0;
}

void editorInsertChar(Editor *e, int c) {
    if (e->cy == e->nrows) {
        editorInsertRow(e, e->nrows, "", 0);
    }
    editorRowInsertChar(e, &e->lines[e->cy], e->cx, c);
    e->cx++;
}

void editorRowInsertChar(Editor *e, Line *line, int at, int c) {
    if (at < 0 || at > line->len) at = line->len;
    line->chars = realloc(line->chars, line->len + 2);
    memmove(&line->chars[at+1], &line->chars[at], line->len - at + 1);
    line->len++;
    line->chars[at] = c;
    editorUpdateRow(e, line);
    e->dirty++;
}

void editorDelChar(Editor *e) {
    if (e->cy == e->nrows) return;
    if (e->cx == 0 && e->cy == 0) return;
    Line *line = &e->lines[e->cy];
    if (e->cx > 0) {
        editorRowDelChar(e, line, e->cx - 1);
        e->cx--;
    }
    else {
        e->cx = e->lines[e->cy - 1].len;
        editorRowAppendString(e, &e->lines[e->cy - 1], line->chars, line->len);
        editorDelRow(e, e->cy);
        e->cy--;
  }
}

void editorRowDelChar(Editor *e, Line *line, int at) {
    if (at < 0 || at >= line->len) return;
    memmove(&line->chars[at], &line->chars[at + 1], line->len - at);
    line->len--;
    editorUpdateRow(e, line);
    e->dirty++;
}

void editorRowAppendString(Editor *e, Line *line, char *s, size_t len) {
    line->chars = realloc(line->chars, line->len + len + 1);
    memcpy(&line->chars[line->len], s, len);
    line->len += len;
    line->chars[line->len] = '\0';
    editorUpdateRow(e, line);
    e->dirty++;
}

void editorScroll(Editor *e) {
    e->rx = 0;
    if (e->cy < e->nrows) {
        e->rx = editorRowCxToRx(e, &e->lines[e->cy], e->cx);
    }
    if (e->cy < e->rowoff) {
        e->rowoff = e->cy;
    }
    if (e->cy >= e->rowoff + e->screenrows) {
        e->rowoff = e->cy - e->screenrows + 1;
    }
    if (e->rx < e->coloff) {
        e->coloff = e->rx;
    }
    if (e->rx >= e->coloff + e->screencols) {
        e->coloff = e->rx - e->screencols + 1;
    }
}

int editorRowCxToRx(Editor *e, Line *line, int cx) {
    (void) e;
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++) {
        if (line->chars[j] == '\t')
            rx += (CEDI_TAB_STOP - 1) - (rx % CEDI_TAB_STOP);
        rx++;
    }
    return rx;
}

char *editorPrompt(Editor *e, char *prompt) {
    size_t bufsize = 128;
    char *buf = malloc(bufsize);
    size_t buflen = 0;
    buf[0] = '\0';
    while (1) {
        editorSetStatusMessage(e, prompt, buf);
        editorRefreshScreen(e);
        int c = editorReadKey(e);
        if (c == K_DEL || c == CTRL_KEY('h') || c == K_BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        }
        else if (c == '\x1b') {
            editorSetStatusMessage(e, "");
            free(buf);
            return NULL;
        }
        else if (c == '\r') {
            if (buflen != 0) {
                editorSetStatusMessage(e, "");
                return buf;
            }
        } 
        else if (!iscntrl(c) && c < 128) {
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                buf = realloc(buf, bufsize);
            }
            buf[buflen++] = c;
            buf[buflen] = '\0';
        }
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