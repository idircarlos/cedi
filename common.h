#ifndef COMMON_H_
#define COMMON_H_

#define DIE(s) die(__FILE__, __LINE__, s)
#define CEDI_VERSION "0.0.1"
#define CEDI_TAB_STOP 4
#define CEDI_DEFAULT_STATUS_MESSAGE "Help: Ctrl-S = save | Ctrl-Q = quit"

void die(char *file, int line, char *s);

#endif // COMMON_H_