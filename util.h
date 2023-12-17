#ifndef UTIL_H_
#define UTIL_H_

int isSeparator(int c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[]{};", c) != NULL;
}

#endif // UTIL_H_