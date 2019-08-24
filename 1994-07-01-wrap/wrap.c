/* fmt.c

   eric
   1 mar 94
*/

/* philosophies at work!

   1. lines should be as close as possible to the ideal
   2. lines should be as close as possible to the preceding line
   3. the last line is a special case and only matters if it's
      really really short
   4. 5 chars of slop in either direction (down from 10) is
      about as much as we can do keeping things reasonably
      consistent-looking
*/

#include <stdio.h>
#include <stdlib.h>

struct line {
  struct line *next;
  char *text;
};
typedef struct line line;

/* globals */

line masterHeader;
char aLine[2000];

int getNextLine (FILE *theFile) {
  char *blah;

  blah = fgets (aLine, 1999, theFile);
  if (blah == 0) return 0;
  else {
    if (aLine[strlen(aLine) - 1] == '\n') aLine[strlen(aLine) - 1] = 0;
    return strlen(aLine) + 1;
  }
}

die (char *foo) {
  fprintf (stderr, foo);
  exit (1);
}


char quotedStuff (char *theString) {
  char *realStart = theString;

  if (*theString == '>' && theString[1] == 'F' && theString[2] == 'r' &&
      theString[3] == 'o' && theString[4] == 'm' && theString[5] == ' ') {
    theString[0] = 'F';
    theString[1] = 'r';
    theString[2] = 'o';
    theString[3] = 'm';
    theString[4] = ' ';
    return 0;
  } /* special case for bloody quoted froms... */

  while (*theString && (*theString != ' ')) {
    switch (*theString) {
    case '#':
    case '$':
    case '%':
    case '+':
    case '<':
    case '=':
    case '>':
/*    case ':': */
    case '\\':
    case '^':
    case '|':
    case '\t':
      return 1;
      break;
    case ':':  /* special case for headers */
      if (*realStart >= 'A' && *realStart <= 'Z') return 1;
      break;
    }
    theString++;
  }
  return 0;
}

void strip (char *source, char *dest) {
  char prev = ' ';

  while (*source) {
    if (*source != ' ') {
      *dest = *source;
      prev = *source;
      source++;
      dest++;
    } else {
      if (prev == ' ') source++;
      else {
	*dest = *source;
	prev = *source;
	source++;
	dest++;
      }
    }
  }
  *dest = 0;

}

void lose (char *what, int how) {
  char foo[200];

  if (how > 200) die ("too long!\n");
  strncpy (foo, what, 199);
  foo[how] = 0;
  printf ("%s\n", foo);
}

void printAs (char *whatString, int wid) {
  int tempWid;

  while (strlen (whatString) > wid) {
    tempWid = wid;
    while ((whatString[tempWid] != ' ') &&
	   (whatString[tempWid] != '-') &&
	   (tempWid > 0)) tempWid--;
    if (tempWid == 0) {
      lose (whatString, wid);
      whatString += wid;
    } else {
      tempWid += 1;
      lose (whatString, tempWid);
      whatString += tempWid;
    }
    while (*whatString == ' ') whatString++;
  }
  printf ("%s\n", whatString);
}

int findBad (char *whatString, int wid, int ideal) {
  int badness = 0;
  int tempWid;
  static int prev = 0;

  while (strlen (whatString) > wid) {
    tempWid = wid;
    while ((whatString[tempWid] != ' ') &&
           (whatString[tempWid] != '-') &&
           (tempWid > 0)) tempWid--;
    if (tempWid == 0) {
      whatString += wid;
      tempWid = wid + 20;
    } else {
      tempWid += 1;
      whatString += tempWid;
    }
    if (prev != 0) badness += abs (tempWid - prev);
    prev = tempWid;
    badness += abs (tempWid - ideal);
    while (*whatString == ' ') whatString++;
  }
/*  badness += abs (strlen (whatString) - prev);
  badness += abs (strlen (whatString) - ideal); */

/* last line of para is a special case -- we only care
   if it's a widow. */

  if (strlen (whatString) < wid / 3) badness += 30;

/*  printf ("%d was %d bad.\n", wid, badness); */
  return badness;
}

void fmtPara (char *theString, int len) {
  char otherString[10240];
  int badness[21]; /* for relative length i, badness[i+10] */
  int best = 967000;
  int whichbest = 999;
  int i;

  strip (theString, otherString);

/*  printAs (otherString, len);
  return; */

  for (i = 5; i < 17; i++) {
    badness[i] = findBad (otherString, len - 10 + i, len);
  }
  strip (theString, otherString);
/*  printf ("otherString is %s\n", otherString); */
  for (i = 5; i < 17; i++) {
    if (badness[i] < best) {
      best = badness[i];
      whichbest = i;
    }
  }
/*  printf ("using %d\n", len - 10 + whichbest); */
  printAs (otherString, len - 10 + whichbest);
}

main (int argc, char *argv[]) {
  line *current;
  line *newHeader;

  char paragraph[10240];
  int maxLen = 70;

  strcpy (paragraph, "");

  masterHeader.next = 0;
  current = &masterHeader;

  while (argc > 1) {
    if (argv[1][0] == '-') {
      sscanf (argv[1]+1, "%d", &maxLen);
    } else {
      if (freopen (argv[1], "r", stdin) == 0) {
	fprintf (stderr, "%s: can't open %s\n", argv[0], argv[1]);
	exit(1);
      }
      
      if (argc > 2) while (getNextLine(stdin) != 0) {
	newHeader = (line *) malloc (sizeof (line));
	if (newHeader == 0) {
	  die ("couldn't allocate memory for a line.");
	}
	newHeader->text = (char *) malloc (strlen (aLine) + 2);
	if (newHeader->text == 0) {
	  die ("couldn't allocate memory for the text.");
	}
	newHeader->next = current->next;
	strcpy (newHeader->text, aLine);
	current->next = newHeader;
	current = newHeader;
      }
    }
    argv++;
    argc--;
  }
  while (getNextLine(stdin) != 0) {
    newHeader = (line *) malloc (sizeof (line));
    if (newHeader == 0) {
      die ("couldn't allocate memory for a line.");
    }
    newHeader->text = (char *) malloc (strlen (aLine) + 2);
    if (newHeader->text == 0) {
      die ("couldn't allocate memory for the text.");
    }
    newHeader->next = current->next;
    strcpy (newHeader->text, aLine);
    current->next = newHeader;
    current = newHeader;
  }
      
  current = masterHeader.next;
  while (current) {
    if (strcmp (current->text, "") == 0) {
      if (paragraph[0] != 0) fmtPara (paragraph, maxLen);
      strcpy (paragraph, "");
      printf ("\n");
    } else if (quotedStuff (current->text)) {
      if (paragraph[0] != 0) fmtPara (paragraph, maxLen);
      strcpy (paragraph, "");
      printf ("%s\n", current->text);
    } else {
      strcat (paragraph, " ");
      strcat (paragraph, current->text);
    }
    current = current->next;
  }
  if (strcmp (paragraph, "") != 0) {
    fmtPara (paragraph, maxLen);
  }
}

/* yeah, i know i never disposed of my linked list.
   memory leaks are fun.
*/
