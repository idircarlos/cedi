cedi: cedi.c editor.h editor.c syntax.h syntax.c util.h util.c
	$(CC) cedi.c editor.c syntax.c util.c -o cedi -Wall -Wextra -pedantic -std=c99
