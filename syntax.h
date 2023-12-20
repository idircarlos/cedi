#ifndef SYNTAX_H_
#define SYNTAX_H_

#include <string.h>
#include <ctype.h>

#define RED 31
#define YELLOW 32
#define GREEN 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37

typedef enum {
    HL_NORMAL = 0,
    HL_SLCOMMENT,
    HL_MLCOMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
} HighlightType;

typedef struct {
    int r, g, b; 
} RGBColor;


typedef enum {
    HL_HIGHLIGHT_NUMBERS = 1<<0,
    HL_HIGHLIGHT_STRINGS = 1<<1,
    HL_HIGHLIGHT_COMMENTS = 1<<2,
} SyntaxHLFlag;

typedef struct {
    char *filetype;
    char **filematch;
    char **keywords;
    char *singleline_comment_start;
    char *multiline_comment_start;
    char *multiline_comment_end;
    SyntaxHLFlag flags;
} EditorSyntax;

int syntaxToColor(int hl);
int syntaxGetFlag(EditorSyntax *syntax, SyntaxHLFlag flag);
EditorSyntax *syntaxFindHighlight(const char *filename);

#endif // SYNTAX_H_