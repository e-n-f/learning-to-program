#include <stdio.h>
#include <string.h>
#include "src/fgetl.h"

#define TAB 8

void spaces (int num) {
	while (num) {
		putchar (' ');
		num--;
	}
}

char *expand (char *src) {
	static char *tmp = 0;
	char *here, *ret;
	int pos;

	if (tmp) free (tmp);
	tmp = (char *) calloc (strlen (src) * TAB, sizeof (char));

	ret = tmp;
	here = tmp;

	pos = 0;
	while (*src) {
		if (*src == '\t') {
			*(here++) = ' ';
			pos++;

			while ((pos % 8) != 0) {
				*(here++) = ' ';
				pos++;
			}
		} else if (*src == '\n') {
			;
		} else {
			*(here++) = *src;
			pos++;
		}
		src++;
	}
	return ret;
}

char *rmlead (char *src) {
	while (*src && *src == ' ') src++;
	return src;
}

char *rmtail (char *src) {
	char *orig = src;

	if (strlen (src) == 0) return src;

	while (*src) src++;
	src--;
	while (*src == ' ') {
		*src = 0;
		src--;
	}
	return orig;
}

void center (FILE *f, int width) {
	char *line;

	while (line = fgetl (f)) {
		line = expand (line);
		line = rmlead (line);
		line = rmtail (line);

		if (strlen (line) > width) {
			printf ("%s\n", line);
		} else {
			spaces ((width - strlen (line)) / 2);
			printf ("%s\n", line);
		}
	}
}

int main (int argc, char **argv) {
	char *argv0 = argv[0];
	int width = 80;

	if (argc > 1 && argv[1][0] == '-') {
		width = atoi (argv[1] + 1);
		argc--; argv++;
	}

	if (argc > 1) {
		while (argc > 1) {
			FILE *this = fopen (argv[1], "r");
			if (this) {
				center (this, width);
				fclose (this);
			} else {
				fprintf (stderr, "%s: can't open %s\n", argv0, argv[1]);
				perror (argv0);
			}
			argv++; argc--;
		}
	} else {
		center (stdin, width);
	}
}
