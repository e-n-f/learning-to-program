/* num.c
*/

#include <stdio.h>
#include <stdlib.h>

struct oneFile {
  char name[256];
  struct oneFile *next;
};
typedef struct oneFile oneFile;

void scoot (int where) {
  while (where > -5) {
    printf (" ");
    where--;
  }
}

void printLine (char *str, int num, char wrap) {
  char part[80];
  char first = 1;
  int column = 0;

  for (;;) {
    if (wrap == 0 || strlen (str) <= 72-column) {
      if (first) {
	printf ("%6d %s", num, str);
	while (str[column] == ' ') column++;
	first = 0;
      } else {
	scoot (column);
	printf ("\\ %s", str);
      }
      break;
    }
    strncpy (part, str, 72-column);
    part[72-column] = 0;
    str += 72-column;
    if (first) {
      printf ("%6d %s\n", num, part);
      while (part[column] == ' ') column++;
      first = 0;
    } else {
      scoot (column);
      printf ("\\ %s\n", part);
    }
  }
}

void stretch (char *source, char *dest) {
  int index = 0;

  while (*source) {
    if (*source != '\t') {
      dest[index] = *source;
      source++;
      index++;
    } else {
      dest[index] = ' ';
      index++;
      while (index % 8) {
	dest[index] = ' ';
	index++;
      }
      source++;
    }
  }
  dest[index] = 0;
}

void doit (FILE *source, char wrap, int start, int end) {
  char bigLine[2000];
  char biggerLine[2100];
  int line = 0;

  while (fgets (bigLine, 1999, source) != NULL) {
    line++;
    stretch (bigLine, biggerLine);
    if (end) if (line > end) break;
    if (line >= start) {
      printLine (biggerLine, line, wrap);
    }
  }
}

main (int argc, char *argv[]) {
  oneFile filenames;
  char wrap = 0;
  int rangeStart = 0;
  int rangeEnd = 0;

  oneFile *currentFile, *newFile;
  char *progName = argv[0];

  filenames.next = 0;
  currentFile = &filenames;

  while (argc > 1) {
    if (argv[1][0] == '-') {
      if (argv[1][1] == 'w') {
	wrap = 1;
      } else if (argv[1][1] == 's') {
	argv++;
	argc--;
	if (sscanf (argv[1], "%d", &rangeStart) != 1) {
	  fprintf (stderr, "%s: start argument '%s' doesn't make sense.\n",
		   progName, argv[1]);
	  exit (1);
	}
      } else if (argv[1][1] == 'e') {
	argv++;
	argc--;
	if (sscanf (argv[1], "%d", &rangeEnd) != 1) {
          fprintf (stderr, "%s: end argument '%s' doesn't make sense.\n",
                   progName, argv[1]);
          exit (1);
        }
      } else if (argv[1][1] == 'a') {
	int near;

        argv++;
        argc--;
        if (sscanf (argv[1], "%d", &near) != 1) {
          fprintf (stderr, "%s: around argument '%s' doesn't make sense.\n",
                   progName, argv[1]);
          exit (1);
        }
	rangeStart = near - 10;
	rangeEnd = near + 10;
      } else {
	fprintf (stderr, "%s: baffling argument %s.\n",
		 progName, argv[1]);
	exit (1);
      }
      argv++;
      argc--;
    } else {
      /* is a filename */

      newFile = (oneFile *)malloc (sizeof (oneFile));
      newFile->next = currentFile->next;
      currentFile->next = newFile;
      strcpy (currentFile->name, argv[1]);
      currentFile = newFile;

      argv++;
      argc--;
    }
  }
  currentFile = &filenames;
  
  if (currentFile->next) while (currentFile->next) {
    FILE *theFile;

    theFile = fopen (currentFile->name, "r");
    if (theFile) {
      doit (theFile, wrap, rangeStart, rangeEnd);
      fclose (theFile);
    } else {
      fprintf (stderr, "%s: unable to read %s.\n",
	       progName, currentFile->name);
    }
    currentFile = currentFile->next;
  } else doit (stdin, wrap, rangeStart, rangeEnd);
}

