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

char *strchrs(const char* str, const char *char_set, int off, int left_to_right) {
    int str_size = strlen(str);
    int set_size = strlen(char_set);
    const char *ptr;
    if (left_to_right) {
        str_size -= off; 
        ptr = str + off;
        for (int i = 0; i < str_size; i++) {
            for (int j = 0; j < set_size; j++) {
                if (ptr[i] == char_set[j]) {
                    LOG_DEBUG("%s %c", ptr, char_set[i]);
                    return strchr(ptr, char_set[j]);
                }
            }
        }
    }
    else {
        ptr = str;
        for (int i = off; i >= 0; i--) {
            for (int j = 0; j < set_size; j++) {
                LOG_DEBUG("Evaluating: %c %c", ptr[i], char_set[j]);
                if (ptr[i] == char_set[j]) {
                    LOG_DEBUG("%s %c", ptr, char_set[j]);
                    return strchr(ptr, char_set[j]);
                }
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