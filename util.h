#ifndef UTIL_H_
#define UTIL_H_

#define DIE(s) die(__FILE__, __LINE__, s)
#define CEDI_VERSION "1.0.0"
#define CEDI_TAB_STOP 4
#define CEDI_DEFAULT_STATUS_MESSAGE "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find"

void die(char *file, int line, char *s);
int isSeparator(int c);
const char *strchrs(const char* str, const char *char_set, const int off);
const char *strrchrs(const char* str, const char *char_set, const int off);
char *strrev(char *str);
// Checks if the cursor x,y its between the limits x1,y1 and x2,y2
int betweenRange(int x, int y, int x1, int y1, int x2, int y2);

#endif // UTIL_H_