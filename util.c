#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "util.h"

void die(char *file, int line, char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
    write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
    fprintf(stderr, "%s:%d %s:%s\n", file, line, s, strerror(errno));
    exit(1);
}

int isSeparator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[]{};", c) != NULL;
}