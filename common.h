#ifndef COMMON_H_
#define COMMON_H_

#define DIE(s) die(__FILE__, __LINE__, s)
#define CEDI_VERSION "0.0.1"
#define CEDI_TAB_STOP 4

void die(char *file, int line, char *s);

#endif // COMMON_H_