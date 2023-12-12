#ifndef EDITOR_H_
#define EDITOR_H_

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
    char *chars;
    int len;
} Line;

typedef struct {
    int cx;
    int cy;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    Line *lines;
    int nrows;
    struct termios orig_termios;
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
void editorRefreshScreen(Editor *e);
void editorOpen(Editor *e, const char *filename);
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
void editorAppendRow(Editor *e, char *s, size_t len);
void editorScroll(Editor *e);
void abAppend(ABuf *ab, const char *s, int len);
void abFree(ABuf *ab);

#endif // EDITOR_H_