#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"
#include "common.h"

// Prototypes
void disableRawMode();
void enableRawMode();

static Editor E = {0};

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

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    enableRawMode();
    editorInit(&E);
    if (argc == 2) {
        editorOpen(&E, argv[1]);
    }

    editorSetStatusMessage(&E, "HELP: Ctrl-S = save | Ctrl-Q = quit");

    while (1) {
        editorRefreshScreen(&E);
        editorProcessKeypress(&E);
    }
    return 0;
}