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


typedef enum {
    HL_HIGHLIGHT_NUMBERS = 1<<0
} SyntaxHLFlag;

typedef struct {
    char *filetype;
    char **filematch;
    SyntaxHLFlag flags;
} EditorSyntax;

int syntaxToColor(int hl);
int syntaxGetFlag(EditorSyntax *syntax, SyntaxHLFlag flag);
EditorSyntax *syntaxFindHighlight(const char *filename);

#endif // SYNTAX_H_