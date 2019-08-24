/* mgrep.c
   eric

   very early in the morning
   7 may 94
   and later in the afternoon too
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

char *re_comp();

struct pipefile {
  FILE *fp;
  int ispipe;
};
typedef struct pipefile pipefile;

struct filename {
  char *name;
  struct filename *next;
};
typedef struct filename filename;

char *mgrepname;

int starts (char *full, char *prefix) {
  while (*prefix) {
    if (*full != *prefix) return 0;
    full++;
    prefix++;
  }
  return 1;
}

int ends (char *full, char *suffix) {
  if (strlen (full) < strlen (suffix)) return 0;
  
  full += strlen(full) - strlen (suffix);

  while (*full) {
    if (*full != *suffix) return 0;
    full++;
    suffix++;
  }
  return 1;
}

void fold (char *str) {
  while (*str) {
    if (*str >= 'A' && *str <= 'Z') *str += 32;
    str++;
  }
}

pipefile makefp (FILE *f, int pf) {
  pipefile ret;

  ret.fp = f;
  ret.ispipe = pf;
  return ret;
}

int closefile (pipefile what) {
  if (what.ispipe) return pclose (what.fp);
  else return fclose (what.fp);
}

pipefile openfile (char *name) {
  char str[500];
  struct stat statbuf;
  pipefile ret;

  if (strcmp (name, "-") == 0) return makefp (stdin, 0);

  if (stat (name, &statbuf) == 0) {
    if (S_ISDIR (statbuf.st_mode)) {
      fprintf (stderr, "%s: %s: is a directory\n", mgrepname, name);
      return makefp (0,0);
    }
  } else {
    fprintf (stderr, "%s: couldn't stat %s\n", mgrepname, name);
    return makefp (0,0);
  }

  if (ends (name, ".Z")) {
    strcpy (str, "uncompress -c ");
    strcat (str, name);
    return makefp (popen (str, "r"), 1);
  } else if (ends (name, ".gz")) {
    strcpy (str, "gunzip -c ");
    strcat (str, name);
    return makefp (popen (str, "r"), 1);
  } else {
    return makefp (fopen (name, "r"), 0);
  }
}

main (int argc, char *argv[]) {
  filename *namelist = 0;
  filename *oldtop = 0;

  char *searchstr = 0;
  char *searchret = 0;

  int foldcase = 0;
  int headeronly = 0;
  int textonly = 0;
  int postmarkonly = 0;
  int ismtxt = 0;
  int isscript = 0;

  mgrepname = argv[0];

  if (argc == 1) {
    fprintf (stderr, "Usage: %s [ -ilhtms ] search-string [ filename ... ]\n", mgrepname);
    exit (1);
  }

  while (argc > 1) {
    if (*argv[1] == '-' && argv[1][1] != 0) {
      char *opts = argv[1]+1;
      while (*opts) {
	if (*opts == 'i') foldcase = 1;
	else if (*opts == 'l') postmarkonly = 1;
	else if (*opts == 'h') headeronly = 1;
	else if (*opts == 't') textonly = 1;
	else if (*opts == 'm') ismtxt = 1;
	else if (*opts == 's') isscript = 1;
	else {
	  fprintf (stderr, "%s: unknown option %c\n", mgrepname, *opts);
	  exit (1);
	}
	opts++;
      }
    } else if (searchstr == 0) {
      searchstr = argv[1];
    } else {
      filename *newname;

      newname = (filename*) malloc (sizeof (filename));
      newname->name = argv[1];
      newname->next = 0;

      if (oldtop == 0) {
	oldtop = newname;
	namelist = newname;
      } else {
	oldtop->next = newname;
	oldtop = newname;
      }
    }
    argv++;
    argc--;
  }

  if (searchstr == 0) {
    fprintf (stderr, "Usage: %s [ -ilhtms ] search-string [ filename ... ]\n", mgrepname);
    exit (1);
  }

  if (foldcase) fold (searchstr);

  searchret = re_comp (searchstr);
  if (searchret != 0) {
    fprintf (stderr, "%s: ", mgrepname);
    if ((int) searchret != -1) fprintf (stderr, searchret);
    else fprintf (stderr, "Internal error");
    fprintf (stderr, " in regular expression handler\n");
    exit (1);
  }

  if (namelist == 0) {
    namelist = (filename*) malloc (sizeof (filename));
    namelist->next = 0;
    namelist->name = "-";
  }

  while (namelist) {
    pipefile thefile;

    if ((thefile = openfile (namelist->name)).fp) {
      char str[4000]; /* surely this is enough... */
      char str2[4000];
      char pmark[4000];
      int headers;
      int msgnum;
      int already = 0;
      int printedyet = 0;

      headers = 0;
      msgnum = 0;
      strcpy (pmark, "(not an mbox file?)\n");

      while (fgets (str, 3999, thefile.fp)) {
	if (strcmp (str, "\n") == 0) headers = 0;
	if (ismtxt == 0) {
	  if (starts (str, "From ")) {
	    headers = 1;
	    msgnum++;
	    strcpy (pmark, str);
	    already = 0;
	  }
	} else {
	  if (strstr (str, ";00000")) {
	    headers = 1;
	    msgnum++;
	    strcpy (pmark, str);
	    already = 0;
	  }
	}

	if (headeronly && (headers == 0)) continue;
	if (textonly && (headers == 1)) continue;

	strcpy (str2, str);

	if (str2[strlen(str2)-1] == '\n') str2[strlen(str2)-1] = 0;
	if (foldcase) fold (str2);

	if (re_exec (str2)) {
	  if (isscript) {
	    if (printedyet == 0) {
	      printf ("file %s\n", namelist->name);
	      printedyet = 1;
	    }
	    if (already == 0) {
	      printf ("%d\n", msgnum);
	      already = 1;
	    }
	  } else if (postmarkonly) {
	    if (already == 0) {
	      printf ("%s/%d:%s", namelist->name, msgnum, pmark);
	      already = 1;
	    }
	  } else {
	    if (msgnum) printf ("%s/%d:%s", namelist->name, msgnum, str);
	    else        printf ("%s:%s", namelist->name, str);
	  }
	}
      }

      closefile (thefile);
    } else {
      fprintf (stderr, "%s: unable to open %s\n", mgrepname, namelist->name);
    }
    namelist = namelist->next;
  }
}
