cedi: cedi.c editor.h editor.c syntax.h syntax.c util.h util.c logger.h logger.c
	$(CC) cedi.c editor.c syntax.c util.c logger.c -o cedi -Wall -Wextra -pedantic -std=c99
