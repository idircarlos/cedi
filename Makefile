cedi: cedi.c editor.h editor.c common.h common.c
	$(CC) cedi.c editor.c common.c -o cedi -Wall -Wextra -pedantic -std=c99
