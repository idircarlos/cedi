#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "util.h"
#include "logger.h"

void die(char *file, int line, char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4); // Clears the entire window
    write(STDOUT_FILENO, "\x1b[H", 3);  // Reposition the cursor to top left
    fprintf(stderr, "%s:%d %s:%s\n", file, line, s, strerror(errno));
    exit(1);
}

int isSeparator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[]{};", c) != NULL;
}

const char *strchrs(const char* str, const char *char_set, const int off) {
    int str_size = strlen(str) - off;
    int set_size = strlen(char_set);
    const char *ptr = str + off;
    for (int i = 0; i < str_size; i++) {
        for (int j = 0; j < set_size; j++) {
            if (ptr[i] == char_set[j]) {
                return strchr(ptr, char_set[j]);
            }
        }
    }
    return NULL;
}

const char *strrchrs(const char* str, const char *char_set, int off) {
    int set_size = strlen(char_set);
    char *strd = (char*)malloc(strlen(str) + 1);
    char *offstrd;
    strcpy(strd, str);
    strd[off] = 0;
    for (int i = off; i >= 0; i--) {
        for (int j = 0; j < set_size; j++) {
            LOG_DEBUG("Evaluating: (%c@%c)", strd[i], char_set[j]);
            if (strd[i] == char_set[j]) {
                offstrd = strrchr(strd, char_set[j]);
                const int pos = offstrd - strd;   // Calculate new pos
                free(strd);
                return str + pos;   // return original pointer + offset
            }
        }
    } 
    return NULL;
}

char *strrev(char *str) {
    char *p1, *p2;
    if (!str || !*str) return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

int betweenRange(int x, int y, int x1, int y1, int x2, int y2) {
    if (y > y1 && y < y2) return 1;
    if (y == y1) { 
        if (y1 == y2) {
            return x >= x1 && x <= x2;
        }
        else {
            return x >= x1;
        }
    }
    if (y == y2) return x <= x2;
    return 0;
}