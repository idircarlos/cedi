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
    char *render;
    int rlen;
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
void editorRefreshScreen(Editor *e);
void editorOpen(Editor *e, const char *filename);
int getWindowSize(int *rows, int *cols);
int getCursorPosition(int *rows, int *cols);
void editorAppendRow(Editor *e, char *s, size_t len);
void editorUpdateRow(Editor *e, Line *line);
void editorScroll(Editor *e);
int editorRowCxToRx(Editor *e, Line *line, int cx);

void abAppend(ABuf *ab, const char *s, int len);
void abFree(ABuf *ab);

#endif // EDITOR_H_