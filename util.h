#ifndef UTIL_H_
#define UTIL_H_

#define DIE(s) die(__FILE__, __LINE__, s)
#define CEDI_VERSION "0.0.1"
#define CEDI_TAB_STOP 4
#define CEDI_DEFAULT_STATUS_MESSAGE "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find"

void die(char *file, int line, char *s);
int isSeparator(int c);
char *strchrs(const char* str, const char *char_set, int off, int left_to_right);
char *strrev(char *str);

#endif // UTIL_H_