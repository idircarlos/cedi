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
    int cx;
    int cy;
    int screenrows;
    int screencols;
    struct termios orig_termios;
} Editor;

typedef struct {
    char *b;
    int len;
} ABuf;

int editorReadKey(Editor *e);
void editorMoveCursor(Editor *e, int key);
void editorProcessKeypress(Editor *e);
void editorDrawRows(Editor *e, ABuf *ab);
void editorRefreshScreen(Editor *e);
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
void abAppend(ABuf *ab, const char *s, int len);
void abFree(ABuf *ab);
void editorInit(Editor *e);

#endif // EDITOR_H_