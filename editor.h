#ifndef EDITOR_H_
#define EDITOR_H_

#include "syntax.h"

typedef enum {
    K_BACKSPACE = 127,
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
    int idx;
    char *chars;
    int len;
    char *render;
    int rlen;
    unsigned char *hl;  // Highlighting
    int hl_open_comment;
} Line;

typedef struct {
    int cx, cy;     // Cursor x,y
    int rx;         // Cursor render x
    int rowoff;     // Row offset for scroll
    int coloff;     // Col offset for scroll
    int screenrows; // Editor rows
    int screencols; // Editor cols
    Line *lines;    // Lines of the editor
    int nrows;      // Number of lines
    int dirty;       // Is dirty
    char *filename; // File name
    char statusmsg[80];
    time_t statusmsg_time;
    EditorSyntax *syntax;   // Syntax of the editor
    struct termios orig_termios;    // Termios status
} Editor;

typedef struct {
    char *b;
    int len;
} ABuf;

void editorInit(Editor *e);
int editorReadKey(Editor *e);
void editorMoveCursor(Editor *e, int key);
void editorProcessKeypress(Editor *e);
void editorDrawRows(Editor *e, ABuf *ab);
void editorDrawStatusBar(Editor *e, ABuf *ab);
void editorRefreshScreen(Editor *e);
void editorSetStatusMessage(Editor *e, const char *fmt, ...);
char *editorRowsToString(Editor *e, int *buflen);
void editorOpen(Editor *e, const char *filename);
void editorSave(Editor *e);
void editorFind(Editor *e);
void editorInsertRow(Editor *e, int at, char *s, size_t len);
void editorFreeRow(Editor *e, Line *line);
void editorDelRow(Editor *e, int at);
void editorUpdateRow(Editor *e, Line *line);
void editorInsertNewline(Editor *e);
void editorInsertChar(Editor *e, int c);
void editorRowInsertChar(Editor *e, Line *line, int at, int c);
void editorDelChar(Editor *e);
void editorRowDelChar(Editor *e, Line *line, int at);
void editorRowAppendString(Editor *e, Line *line, char *s, size_t len);
void editorScroll(Editor *e);
int editorRowCxToRx(Editor *e, Line *line, int cx);
int editorRowRxToCx(Editor *e, Line *line, int rx);
char *editorPrompt(Editor *e, char *prompt, void (*callback)(Editor *, char *, int));
void editorUpdateSyntax(Editor *e, Line *line);
void editorSelectSyntaxHighlight(Editor *e);

int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);

void abAppend(ABuf *ab, const char *s, int len);
void abFree(ABuf *ab);

#endif // EDITOR_H_