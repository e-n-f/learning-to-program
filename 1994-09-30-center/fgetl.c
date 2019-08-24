#include <stdio.h>
#include <stdlib.h>

#include "fgetl.h"

/* INITBUFSIZE defines the number of characters to be allocated for
   text storage the first time fgetl() is called.

   BUFSIZEINCR defines the number of characters by which this size
   is to be incremented each time the previous buffer size proves
   to have been inadequate.
*/

#define INITBUFSIZE 100
#define BUFSIZEINCR 100

/* fgl_alloc uses calloc() to allocate enough space to hold
   size characters.  if calloc() fails, fgl_alloc prints
   an error message to the standard output and does not
   return.
*/

char *fgl_alloc (int size) {
	char *newbuf = (char *) calloc (size, sizeof (char));

	if (newbuf == 0) {
		fprintf (stderr, "fgetl: could not allocate enough memory ");
		fprintf (stderr, "to read a line of text.  sorry.\n");
		exit (1);
	}
	return newbuf;
}

/* fgetl gets a text line of arbitrary length from the stream
   file *which.  GNU seems to prefer text input routines which
   allocate memory each time, to be free()d by the calling
   routine, but, to be more in keeping with typical programs
   which read a zillion lines most of which don't need to be
   kept around, fgetl returns a pointer to a STATIC buffer
   whose contents are overwritten with each call.  callers who
   want their strings to stick around can use strdup() to make
   a copy.

   like fgets(), the \n at the end of a line is PRESERVED.
*/

char *fgetl (FILE *which) {
	static char *line = 0;
	static int len = 0;      /* length of string line[] */

	if (line == 0) {  /* if this is the first time, make a buffer */
		line = fgl_alloc (INITBUFSIZE);
		len = INITBUFSIZE;
	}

	{
		int c;                /* each character read. */
		int currentlen = 0;   /* length of the string so far */

		*line = 0;            /* erase last time's string */

		while ((c = getc (which)) != EOF) {

			/* check to see if we've fallen off the end of a line.
			   if we have, allocate a new string that's
			   BUFSIZEINCR characters longer, copy the contents of
			   the old string into it, and delete the old string.

			   the check says "currentlen + 2" because we have to
			   provide space both for this character we're adding
			   now and for the \0 to be added at the end of the line.
			*/

			if (currentlen + 2 > len) { 
				char *newline = fgl_alloc (len + BUFSIZEINCR);

				strcpy (newline, line);
				free (line);
				line = newline;
				len += BUFSIZEINCR;
			}

			line [currentlen] = c;
			line [++currentlen] = 0;

			if (c == '\n') break;
		}
		if (currentlen == 0) return 0;  /* no newline, so is EOF */
	}
	return line;
}
