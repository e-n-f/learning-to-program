/* pac man
   for termcap

   eric fischer
   8-9 aug 94
*/

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <sgtty.h>

int TICKS = 125;
#define QUANTUM ((TICKS/2)+1)

int DTICKS = 125;
int dnow = 1;

#define HOMEX 34
#define HOMEY 11

#define FRUITX 37
#define FRUITY 8

char *screenname = "/e4/enf1/lib/pac.screen";
char *titlename = "/e4/enf1/lib/pac.title";
char *scorename = "/e4/enf1/lib/pac.scores";

char who[10][80];
int hi[10];

void getscores() {
	FILE *scfile;
	char s[80];
	int i;

	for (i = 0; i < 10; i++) {
		strcpy (who[i], "\n");
		hi[i] = 0;
	}
	scfile = fopen (scorename, "r");
	if (scfile) {
		while (fgets (s, 80, scfile)) {
			int sc = atoi(s);
			for (i = 0; i < 10; i++) {
				if (sc > hi[i]) {
					int j;

					for (j = 10; j > i; j--) {
						hi[j] = hi[j-1];
						strcpy (who[j], who[j-1]);
					}
					hi[i] = sc;
					strcpy (who[i], s);
					break;
				}
			}
		}
		fclose (scfile);
	}
}

/* yeah, i know we should call ioctl() instead, but... */

struct sgttyb terms;

void cbreak() {
	gtty (0, &terms);
	terms.sg_flags |= CBREAK;
	stty (0, &terms);
}

void nocbreak() {
	gtty (0, &terms);
	terms.sg_flags &= ~CBREAK;
	stty (0, &terms);
}

void echo() {
	gtty (0, &terms);
	terms.sg_flags |= ECHO;
	stty (0, &terms);
}

void noecho() {
	gtty (0, &terms);
	terms.sg_flags &= ~ECHO;
	stty (0, &terms);
}

static char *TERM;
static char *cm, *cl, *ce;
static char bp[1024], buf[1024];
char *boof = buf;

char *basename (char *foo) {
	char *ret = foo;
	while (*foo) {
		if (*foo == '/') ret = foo+1;
		foo++;
	}
	return ret;
}

void initcap() {
	TERM = (char *) getenv ("TERM");
	tgetent (bp, TERM);
	cm = (char *) tgetstr ("cm", &boof);
	if (cm == 0) {
		fprintf (stderr, "Your terminal can't move the cursor.  No games for you.\n");
		exit (1);
	}
	cl = (char *) tgetstr ("cl", &boof);
	if (cl == 0) {
		fprintf (stderr, "You can move the cursor but not clear the screen?!?\n");
		fprintf (stderr, "That's really dumb.\n");
		exit (1);
	}
	ce = (char *) tgetstr ("ce", &boof);
	if (ce == 0) {
		fprintf (stderr, "And you can't clear to the end of a line.  Oh well.\n");
	}
}

int waitforchar (int ticks) {

/* stolen from vim's unix.c */

	struct timeval tv;
	fd_set fdset;

	if (ticks >= 0) {
		tv.tv_sec = ticks/1000;
		tv.tv_usec = (ticks % 1000) * (1000000/1000);
	}
	FD_ZERO (&fdset);
	FD_SET (0, &fdset);
	return (select (1, &fdset, NULL, NULL, (ticks >= 0) ? &tv : NULL));
}

int even = 0;
int odd = 0;
int score = 0;
int lives = 3;
int dots, wasdots;

int fruit = 0;

int gamestart = 0;
int havesusp = 0;

int nextkey() {
	int what = 0;
	struct timeval starttime, now;

	gettimeofday (&starttime, 0);

	while (1) {
		unsigned long nowticks, thenticks;

		if (waitforchar(QUANTUM)) what = getchar ();
		gettimeofday (&now, 0);

		nowticks = (long) now.tv_usec/1000 + 1000*(now.tv_sec - gamestart);
		thenticks = (long) starttime.tv_usec/1000 + 1000 * (starttime.tv_sec - gamestart);
		if (nowticks - thenticks > TICKS) break;
		if (havesusp) break;
	}
	havesusp = 0;
	return what;
}

void myputchar (char x) {
	putchar (x);
}

void gotoyx (int y, int x) {
	char *move;

	move = (char *) tgoto (cm, x, y);
	tputs (move, 1, myputchar);
}

void clearscr() {
	tputs (cl, 1, myputchar);
}

void clearln() {
	if (ce) tputs (cl, 1, myputchar);
	else printf ("                                                                               ");
}

char onscreen[24][80];
char shouldscreen[24][80] = { "                                                                                " };
char board[24][80];

void replaceboard() {
	int i;
	for (i = 0; i < 24; i++) {
		strcpy (onscreen[i], board[i]);
	}
}

void fixscreen () {
	int y;
	int x;

	dnow += TICKS;
	if (dnow >= DTICKS) {
		dnow -= DTICKS;
		for (y = 0; y < 23; y++) {
			for (x = 0; x < 79 && onscreen[y][x]; x++) {
				if (onscreen[y][x] != shouldscreen[y][x]) {
					gotoyx (y, x);
					while (x < 79 && (
						onscreen[y][x] != shouldscreen[y][x] ||
						onscreen[y][x+1] != shouldscreen[y][x+1] ||
						onscreen[y][x+2] != shouldscreen[y][x+2] ||
						onscreen[y][x+3] != shouldscreen[y][x+3] ||
						onscreen[y][x+4] != shouldscreen[y][x+4] ||
						onscreen[y][x+5] != shouldscreen[y][x+5]
					)) {
						putchar (onscreen[y][x]);
						shouldscreen[y][x] = onscreen[y][x];
						x++;
					}
				}
			}
		}
		gotoyx (1, 1);
		fflush (stdout);
	}
}

char *keys = " ijkm";

static int pacx = 32, pacy = 8;
static int go = 0;
static int want = 0;

static int gx[4] = { 38, 38, 38, 38 };
static int gy[4] = { 11, 11, 11, 11 };
static int gg[4] = { 0, 0, 0, 0 };
static int blocked[4] = { 0, 0, 0, 0 };
static int dead[4] = { 0, 0, 0, 0 };

static int pellet = 0;

int bblock (int line) {
	int x;
	for (x = pacx; x < pacx+4; x++) {
		if (board[line][x] == '=') return 0;
	}
	return 1;
}

int gbblock (int g, int line) {
	int x, i;
	for (x = gx[g]; x < gx[g]+4; x++) {
		if (board[line][x] == '=') return 0;
		for (i = 0; i < 4; i++) {
			if (gx[i] == x && gy[i] == line) return 0;
		}
	}
	return 1;
}

int lblock (int col) {
	int y;
	for (y = pacy; y < pacy+2; y++) {
		if (board[y][col] == '=') return 0;
	}
	return 1;
}

int glblock (int g, int col) {
	int y, i;
	for (y = gy[g]; y < gy[g]+2; y++) {
		if (board[y][col] == '=') return 0;
		for (i = 0; i < 4; i++) {
			if (gy[i] == y && gx[i] == col) return 0;
		}
	}
	return 1;
}

int canmove (int dir) {
	switch (dir) {
		case 1:
			return bblock (pacy-1);
		break;
		case 2:
			return lblock (pacx-1);
		break;
		case 3:
			return lblock (pacx+4);
		break;
		case 4:
			return bblock (pacy+2);
		break;
	}
	return 1;
}

int gcanmove (int g, int dir) {
	if (dead[g]) return 1;

	switch (dir) {
		case 1:
			return gbblock (g, gy[g]-1);
		break;
		case 2:
			return glblock (g, gx[g]-1);
		break;
		case 3:
			return glblock (g, gx[g]+4);
		break;
		case 4:
			return gbblock (g, gy[g]+2);
		break;
	}
	return 0;
}

void sp (char *dest, char *src) {
	while (*src) {
		*dest++ = *src++;
	}
}

void drawg (int which) {
	char *fir = onscreen[gy[which]] + gx[which];
	char *sec = onscreen[gy[which]+1] + gx[which];

	if (dead[which]) {
		sp (fir+1, "oo");
	} else {
		if (pellet == 0) sp (fir, "/oo\\");
		else if (pellet > 10) sp (fir, "/@@\\");
		else if (even) sp (fir, "/@@\\");
		else sp (fir, "/oo\\");

		sp (sec, "vvvv");
	
		if (onscreen[gy[which]-1][gx[which]+1] == ' ')
			onscreen[gy[which]-1][gx[which]+1] = '_';
		if (onscreen[gy[which]-1][gx[which]+2] == ' ') 
			onscreen[gy[which]-1][gx[which]+2] = '_';
	}
}

void drawfruit () {
	char *fir = onscreen[FRUITY] + FRUITX;
	char *sec = onscreen[FRUITY+1] + FRUITX;
	if (fruit > 10 || (fruit && even)) {
		sp (fir, " /|");
		sp (sec, "O O");
	}
}

void drawpac () {
	char *fir = onscreen[pacy] + pacx;
	char *sec = onscreen[pacy+1] + pacx;
	switch (go) {
		case 0:
			sp (fir, "/  \\");
			sp (sec, "\\__/");
		break;
		case 1: /* up */
			if (even) {
				sp (fir, "/\\/\\");
				sp (sec, "\\__/");
			} else {
				sp (fir, "/| \\");
				sp (sec, "\\__/");
			}
		break;
		case 2: /* left */
			if (even) {
				sp (fir, " \\ \\");
				sp (sec, " /_/");
			} else {
				sp (fir, "/_ \\");
				sp (sec, "\\__/");
			}
		break;
		case 3: /* right */
			if (even) {
				sp (fir, "/ /");
				sp (sec, "\\_\\");
			} else {
				sp (fir, "/ _\\");
				sp (sec, "\\__/");
			}
		break;
		case 4: /* down */
			if (even) {
				sp (fir, "/  \\");
				sp (sec, "\\|_/");
			} else {
				sp (fir, "/  \\");
				sp (sec, "\\/\\/");
			}
		break;
	}
	if (go != 1 || even == 0) {
		if (onscreen[pacy-1][pacx+1] == ' ')
			onscreen[pacy-1][pacx+1] = '_';
		if (onscreen[pacy-1][pacx+2] == ' ') 
			onscreen[pacy-1][pacx+2] = '_';
	}
}

void movepac (int dir) {
	switch (dir) {
		case 1:
			if (even) pacy--;
		break;
		case 2:
			pacx--;
		break;
		case 3:
			pacx++;
		break;
		case 4:
			if (even) pacy++;
		break;
	}
	if (pacx < 0) pacx = 74;
	if (pacx > 74) pacx = 0;
}

void moveg (int g, int dir) {
	switch (dir) {
		case 1:
			if (even) gy[g]--;
		break;
		case 2:
			gx[g]--;
		break;
		case 3:
			gx[g]++;
		break;
		case 4:
			if (even) gy[g]++;
		break;
	}
	if (gx[g] < 0) gx[g] = 74;
	if (gx[g] > 74) gx[g] = 0;
}

int under (int x, int y, char what) {
	int xx, yy;

	for (xx = x; xx < x + 4; xx++) {
		for (yy = y; yy < y + 2; yy++) {
			if (board[yy][xx] == what) return 1;
		}
	}
	return 0;
}

void zap (int x, int y, char what) {
	int xx, yy;

	for (xx = x; xx < x + 4; xx++) {
		for (yy = y; yy < y + 2; yy++) {
			if (board[yy][xx] == what) board[yy][xx] = ' ';
		}
	}
}

int prev (int dir) {
	return (dir - 2) % 4 + 1;
}

int succ (int dir) {
	return dir % 4 + 1;
}

int opp (int dir) {
	return (dir - 3) % 4 + 1;
}

extern char *cuserid();

void drawscore() {
	char scorestr[50];
	static char *me = 0;
	char hiscore[80] = "";

	if (me == 0) me = cuserid(0);

	sprintf (scorestr, "%s %d", me, score);
	sp (board[0] + 1, scorestr);

	if (*hiscore == 0) {
		char temp[80];
		char *blob = temp;

		strcpy (temp, who[0]); /* high scorer */
		while (*blob && *blob != '\t') {
			if (*blob == '/') *blob = ' ';
			blob++;
		}
		*blob = 0;
		sprintf (scorestr, "%15s", temp); 
		strcpy (hiscore, scorestr);
	}
	sp (board[0] + 62, hiscore);
}

void refresh() {
	int i;

	clearscr();
	for (i = 0; i < 24; i++) strcpy (shouldscreen[i], "                                                                               ");
	drawscore();
	replaceboard();
	drawpac();
	for (i = 0; i < 4; i++) drawg (i);
	dnow = DTICKS;
	fixscreen();
	sleep((16 * DTICKS)/1000);
}

void doit (int key) {
	int i;

	for (i = 0; i < 5; i++) if (key == keys[i]) want = i;
	if (key == 'q') lives = 0;
	if (key == 12) refresh();

	if (canmove (want)) go = want;

	if (canmove (go)) movepac (go);
	drawpac ();
	if (under (pacx, pacy, '.')) {
		zap (pacx, pacy, '.');
		score += 10;
		dots--;
		if (dots == (wasdots / 2)) fruit = 75;
		drawscore();
	}

	if (fruit) {
		drawfruit();
		fruit--;
		if (abs (pacx-FRUITX) < 4 && abs (pacy-FRUITY) < 2) {
			score += 150;
			drawscore();
			fruit = 0;
		}
	}

	if (under (pacx, pacy, 'O')) {
		zap (pacx, pacy, 'O');
		pellet = 50;
		score += 50;
		drawscore();
	}

	if (pellet) pellet--;

	odd = (odd + 1) % 3;

	if ((!pellet && odd) || (pellet && even)) {
		for (i = 0; i < 4; i++) {
			if (dead[i]) {
				if (gx[i] < HOMEX) gg[i] = 3;
				else if (gx[i] > HOMEX) gg[i] = 2;
				else if (gy[i] < HOMEY) gg[i] = 4;
				else if (gy[i] > HOMEY) gg[i] = 1;
				else dead[i] = 0;
			} else {
				if (blocked[i] == 0) {
					if (pellet == 0) {
						if (pacy < gy[i] && gcanmove (i, 1)) gg[i] = 1;
						else if (pacy > gy[i] && gcanmove (i, 4)) gg[i] = 4;	
						else if (pacx < gx[i] && gcanmove (i, 2)) gg[i] = 2;
						else if (pacx > gx[i] && gcanmove (i, 3)) gg[i] = 3;
					} else {
						if (pacy < gy[i] && gcanmove (i, 1)) gg[i] = 4;
						else if (pacy > gy[i] && gcanmove (i, 4)) gg[i] = 1;
						else if (pacx < gx[i] && gcanmove (i, 2)) gg[i] = 3;
						else if (pacx > gx[i] && gcanmove (i, 3)) gg[i] = 2;
					}
				} else blocked[i]--;
			}
	
			if (gcanmove (i, gg[i]) == 0) {
				gg[i] = rand() % 4 + 1;
				if (gcanmove (i, gg[i])) blocked[i] = 5;
			}
			if (gcanmove (i, gg[i])) moveg (i, gg[i]);
		}
	}
	for (i = 0; i < 4; i++) {
		drawg (i);
	}
	for (i = 0; i < 4; i++) {
		if (!dead[i] && abs (pacx-gx[i]) < 4 && abs (pacy-gy[i]) < 2) {
			if (pellet) {
				dead[i] = 1;
				score += 100;
				drawscore();
			} else if (dead [i]) {
				;
			} else {
				char *zed = onscreen[pacy-1] + pacx;
				char *fir = onscreen[pacy] + pacx;
				char *sec = onscreen[pacy+1] + pacx;

				replaceboard();
				sp (zed, " __ ");
				sp (fir, "/  \\");
				sp (sec, "\\__/");
				fixscreen();
				nextkey();
				sp (zed, "    ");
				sp (fir, "/\\/\\");
				fixscreen();
				nextkey();
				sp (fir, "____");
				fixscreen();
				nextkey();
				sp (fir, "    ");
				sp (sec, " /\\ ");
				fixscreen();
				sleep(1);
				pacx = 32;
				pacy = 8;
				go = 0;
				want = 0;
				{
					int boo;
					for (boo = 0; boo < 4; boo++) {
						gx[boo] = 38;
						gy[boo] = 11;
					}
				}
				while (waitforchar (2)) getchar();
				lives--;
				sp (board[0] + 56, "     ");
				{
					int i;
					for (i = 0; i < lives; i++) {
						sp (board[0] + 56 + i*2, "C");
					}
				}
			}
		}
	}
}

void comeback();

void besuspended() {
	clearscr();
	fflush (stdout);
	signal (SIGCONT, comeback);
	signal (SIGTSTP, SIG_DFL);
	nocbreak();
	echo();
	kill (0, SIGTSTP);
}

void comeback() {
	signal (SIGTSTP, besuspended);
	havesusp = 1;
	fflush (stdout);
	cbreak();
	noecho();
	refresh();
	ungetc ('~', stdin);  /* put something there so we don't block */
}

char **oldargv;

void usage() {
	fprintf (stderr, "Usage: %s [-s] [-ffilename] [-tticks] [-drate] keys\n", oldargv[0]);
}

void fix (char *what) {
	int done = 0;
	int i;

	for (i = 0; i < 80; i++) {
		if (what[i] == '\t') what[i] = ' ';
		if (what[i] == 0 || what[i] == '\n') done = 1;
		if (done) what[i] = ' ';
	}
	what[78] = '\n';
	what[79] = 0;
}

int main (int argc, char **argv) {
	int i;
	int key;
	char blob[200];
	int splash = 1;

	FILE *title;
	FILE *screen;

	oldargv = argv;
	gamestart = time(0);
	while (argc > 1) {
		if (*argv[1] == '-') {
			if (argv[1][1] == 'f') screenname = argv[1]+2;
		    else if (argv[1][1] == 't') TICKS = atoi (argv[1]+2);
			else if (argv[1][1] == 'd') DTICKS = atoi (argv[1]+2);
			else if (argv[1][1] == 's') splash = 0;
			else {
				fprintf (stderr, "Unknown option %s\n", argv[1]);
				usage();
				exit (1);
			}
		} else if (strlen (argv[1]) == 5) keys = argv[1];
		else {
			fprintf (stderr, "Unknown option %s\n", argv[1]);
			usage();
			exit (1);
		}
		argv++; argc--;
	}
	argv = oldargv;

	screen = fopen (screenname, "r");

	if (screen == 0) {
		fprintf (stderr, "Can't find the screen named %s\n!", screenname);
		exit (1);
	}

	initcap();
	clearscr();

	if (splash) {
		title = fopen (titlename, "r");
		if (title) {
			while (fgets (blob, 200, title)) printf (blob);
			fclose (title);
		}

		printf ("Keys:  up    left   right   down   stop       press [q] to quit\n");
		printf ("       [%c]    [%c]    [%c]     [%c]    [%c]",
			keys[1], keys[2], keys[3], keys[4], keys[0]);
		printf ("       %s \"sulrd\" to change keys\n", basename(argv[0]));
		printf ("\n");
		printf ("Press Return to begin:   ");
		gets (blob);
	}
	getscores();

	cbreak();
	noecho();
	clearscr();
	for (i = 0; i < 23; i++) {
		fgets (board[i], 80, screen);
		fix (board[i]);
		{
			char *foo = board[i];
			while (*foo) {
				if (*foo == '.') dots++;
				foo++;
			}
		}
	}
	fclose (screen);

	wasdots = dots;
	signal (SIGTSTP, besuspended);

	refresh();

	while (lives) {
		even = !even;
		key = nextkey();
		replaceboard();
		doit (key);
		fixscreen();

		if (dots == 0) {
			sleep(1);
			replaceboard();
			drawpac();
			fixscreen();
			sleep(1);
			
			screen = fopen (screenname, "r");
			if (screen == 0) {
				fprintf (stderr, "that's odd -- the screen file was there last time\n");
				exit (1);
			}
			for (i = 0; i < 23; i++) {
				fgets (board[i], 80, screen);
				fix (board[i]);
				strcpy (onscreen[i], board[i]);
				{
					char *foo = board[i];
					while (*foo) {
						if (*foo == '.') dots++;
						foo++;
					}
				}
			}
			fclose (screen);
			pacx = 32; pacy = 8;
			for (i = 0; i < 4; i++) {
				gx[i] = HOMEX;
				gy[i] = HOMEY;
			}
			go = 0;
			want = 0;
			refresh();
		}
	}
	nocbreak();
	echo();
	clearscr();
	if (score > hi[9]) {
		FILE *scfile = fopen (scorename, "a+");
		if (scfile) {
			int foo = time(0);
			fprintf (scfile, "%d/%s\t%s", score, cuserid(0), ctime(&foo));
		}
		fclose (scfile);
	}
	getscores();
	printf ("               High Scores\n\n");
	{
		int i;
		for (i = 0; i < 10; i++) {
			printf ("     %2d %6d %s", i+1, hi[i], basename(who[i]));
		}
	}
	printf ("\n\n\n\n\n");
}
