#include "syntax.h"

static char *C_HL_extensions[] = {".c", ".h", ".cpp", NULL};
static char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",

  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};

static EditorSyntax HLDB[] = {
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//",
    "/*",
    "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS | HL_HIGHLIGHT_COMMENTS
  },
};

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

int syntaxToColor(int hl) {
    switch (hl) {
        case HL_SLCOMMENT:
        case HL_MLCOMMENT: return CYAN;
        case HL_KEYWORD1: return GREEN;
        case HL_KEYWORD2: return YELLOW;
        case HL_STRING: return MAGENTA; 
        case HL_NUMBER: return RED; 
        case HL_MATCH: return BLUE;
        default: return WHITE;
    }
}

int syntaxGetFlag(EditorSyntax *syntax, SyntaxHLFlag flag) {
    return syntax->flags & flag;
}

EditorSyntax *syntaxFindHighlight(const char *filename) {
    char *ext = strrchr(filename, '.');
    for (unsigned int j = 0; j < HLDB_ENTRIES; j++) {
        EditorSyntax *s = &HLDB[j];
        unsigned int i = 0;
        while (s->filematch[i]) {
            int is_ext = (s->filematch[i][0] == '.');
            if ((is_ext && ext && !strcmp(ext, s->filematch[i])) || (!is_ext && strstr(filename, s->filematch[i]))) {
                return s;
            }
            i++;
        }
    }
    return NULL;
}