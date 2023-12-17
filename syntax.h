#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <string.h>
#include <ctype.h>

#define RED 31
#define BLUE 34
#define WHITE 37

typedef enum {
    HL_NORMAL = 0,
    HL_NUMBER,
    HL_MATCH
} HighlightType;

typedef struct {
    int r, g, b; 
} RGBColor;

int syntaxToColor(int hl) {
    switch (hl) {
        case HL_NUMBER: return RED;
        case HL_MATCH: return BLUE;
        default: return WHITE;
    }
}

#endif // SYNTAX_H_