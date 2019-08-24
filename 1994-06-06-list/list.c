/* list.c */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <ctype.h>

void die (char *why) {
	fprintf (stderr, "%s", why);
	exit (1);
}

int colon (char *str) {
	while (*str && *str != ' ') {
		if (*str == ':') return 1;
		str++;
	}
	return 0;
}

void lowcase (char *what) {
	while (*what) {
		if (isupper (*what)) *what += 'a' - 'A';
		what++;
	}
}

void strcatc (char *str, char what) {
	char foo[2];

	foo[0] = what;
	foo[1] = 0;
	strcat (str, foo);
}

char months[13][4] = {"???", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char *parsedate (char *was) {

	static char result[200];

	int month, day, year;
	int hour, minute;
	int tmp;

	month = day = year = hour = minute = 0;

	lowcase (was);
	while (*was) {
		if (colon (was)) {
			sscanf (was, "%d:%d", &hour, &minute);
		} else if (*was == '+' || *was == '-') {
			;
		} else if (sscanf (was, "%d", &tmp) == 1) {
			if (month) {
				if (day == 0) day = tmp;
				else if (year == 0) year = tmp %100;
			} else {
				if (day) {
					if (tmp > 1900) year = tmp %100;
					else {
						month = day;
						day = tmp;
					}
				} else {
					if (tmp > 1900) year = tmp %100;
					else day = tmp;
				}
			}
		} else if (starts (was, "ja")) month = 1;
		else if (starts (was, "fe")) month = 2;
		else if (starts (was, "mar")) month = 3;
		else if (starts (was, "ap")) month = 4;
		else if (starts (was, "may")) month = 5;
		else if (starts (was, "jun")) month = 6;
		else if (starts (was, "jul")) month = 7;
		else if (starts (was, "au")) month = 8;
		else if (starts (was, "se")) month = 9;
		else if (starts (was, "oc")) month = 10;
		else if (starts (was, "no")) month = 11;
		else if (starts (was, "de")) month = 12;

		while (*was && *was != ' ') was++;
		while (*was == ' ') was++;
	}
	sprintf (result, "%2d %s %d %2d:", day, months[month], year, hour);
	strcatc (result, minute/10 + '0');
	strcatc (result, minute%10 + '0');
	return result;
}

/**************** pfile stuff *******************/

struct PFILE {
	FILE *file;
	int ispipe;
};
typedef struct PFILE PFILE;

PFILE *newpfile (FILE *file, int how) {
	PFILE *blah;

	blah = (PFILE *)malloc (sizeof (PFILE));
	if (blah == 0) return 0;

	blah->file = file;
	blah->ispipe = how;
	return blah;
}

int ends (char *big, char *lit) {
	if (strlen (big) < strlen (lit)) return 0;
	big += (strlen (big) - strlen (lit));

	while (*big) {
		if (*big != *lit) return 0;
		big++;
		lit++;
	}
	return 1;
}

PFILE *pfopen (char *gee, char *how) {
	char cmd[2000]; /* yes, it's arbitrary, dammit... */
	FILE *try;

	if (strcmp (gee, "-") == 0) {
		return newpfile (stdin, 1);
	} else if (ends (gee, ".Z")) {
		strcpy (cmd, "uncompress -c ");
		strcat (cmd, gee);
		if (try = popen (cmd, how)) return newpfile (try, 1);
		else return 0;
	} else if (ends (gee, ".gz")) {
		strcpy (cmd, "gunzip -c ");
		strcat (cmd, gee);
		if (try = popen (cmd, how)) return newpfile (try, 1);
		else return 0;
	} else {
		if (try = fopen (gee, how)) return newpfile (try, 0);
		else return 0;
	}
}

int pfclose (PFILE *pfile) {
	int suc;

	if (pfile->ispipe) suc = pclose (pfile->file);
	else suc = fclose (pfile->file);

	free (pfile);
	return suc;
}

/**************** end pfile stuff ***************/

struct name {
	char *text;
	struct name *next;
};
typedef struct name name;

name *insert (char *str, name *old) {
	if (old == 0) {
		name *this;

		this = (name *)malloc (sizeof (name));
		if (this == 0) die ("malloc failed!\n");
		this->text = str;
		this->next = 0;
		return this;
	} else {
		name *this;

		this = old;
		while (this->next) this = this->next;

		this->next = (name *)malloc (sizeof (name));
		if (this->next == 0) die ("malloc failed!\n");
		this = this->next;

		this->text = str;
		this->next = 0;
		return old;
	}
}


void toss (char *what, int len) {
	while (*what == ' ') what++;
	while (len) {
		if (*what) {
			putchar (*what);
			what++;
		} else putchar (' ');
		len--;
	}
}

int starts (char *big, char *little) {
	while (*little) {
		if (*big != *little) return 0;
		big++;
		little++;
	}
	return 1;
}

void stripcr (char *str) {
	if (str[strlen(str)-1] == '\n') str[strlen(str)-1] = 0;
}

int main (int argc, char *argv[]) {
	char *progname = argv[0];
	name *names = 0;
	char spool[300];

	while (argc > 1) {
		names = insert (argv[1], names);
		argc--;
		argv++;
	}

	strcpy (spool, "/usr/spool/mail/");
	strcat (spool, getpwuid (getuid())->pw_name);
	if (names == 0) names = insert (spool, 0);

	while (names) {
		PFILE *thefile;

		printf ("%s:\n", names->text);

		if (thefile = pfopen (names->text, "r")) {
			int mbox;
			char str[2000];
			int num = 0;

			int firstc = getc (thefile->file);
			ungetc (firstc, thefile->file);

			if (firstc == 'F') mbox = 1;
			else mbox = 0;

			while (fgets (str, 2000, thefile->file)) {
				if (((mbox == 1) && starts (str, "From ")) ||
				    ((mbox == 0) && strstr (str, ";0000"))) {

					char froml[2000]="", subl[2000]="", datel[2000]="";
					char statl[2000]="";

					if (mbox) {
						char *blah;
						strcpy (froml, "From: ");
						strcat (froml, str+5);

						blah = froml+6;
						while (*blah && *blah != ' ') blah++;
						*blah = 0;

						strcpy (datel, str);
					} else {
						strcpy (datel, str);
					}

					while (fgets (str, 2000, thefile->file)) {
						if (strcmp ("\n", str) == 0) {
							++num;
							toss (statl+8, 3);
							printf ("%3d ", num);
							printname (froml+6, 25);
							printf ("  ");
							printsubject (subl+9, 25);
							printf ("  ");
							printf ("%s\n", parsedate (datel+6));
							break;
						} else {
							stripcr (str);
							if (starts (str, "From: ")) strcpy (froml, str);
							if (starts (str, "Subject: ")) strcpy (subl, str);
							if (starts (str, "Date: ")) strcpy (datel, str);
							if (starts (str, "Status: ")) strcpy (statl, str);
						}
					}
				}
			}
			pfclose (thefile);
		} else {
			fprintf (stderr, "%s: failed to open %s\n", progname, names->text);
		}
		names = names->next;
	}
}
