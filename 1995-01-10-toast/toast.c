#include <stdio.h>
#include <stdlib.h>
#include <sgtty.h>

#define PICLINES 8
#define FRAMES 6
#define MAXTOASTERS 30

int speeds[] = { 0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800,
	2400, 4800, 9600, 19200, 38400};

int numtoasters = 5;

int LINES = 24;
int COLS = 80;
int widest;

int charsleft;
int charsused;

struct toaster {
	int x;
	int y;
	int frame;
};
typedef struct toaster toaster;

toaster toasters[MAXTOASTERS];

char *toast[4][PICLINES];
int seq[FRAMES];

char tcapent[2048], tcapjunk[2048];
char *space = tcapjunk;

char *TERM;
char *clearscreen;
char *moveto;
char *initstring;
char *sane;
char *cleareol;

char *tgetent(), *tgetstr(), *tgoto();
void die (char *);

int termspeed = 9600;

char *onscreen, *wantscreen;

int
framespersec()
{
	return 8;
}

void
makescreen()
{
	onscreen = malloc (LINES * COLS * sizeof (char));
	wantscreen = malloc (LINES * COLS * sizeof (char));

	if (onscreen == 0 || wantscreen == 0) {
		die ("Couldn't allocate memory for the screen!\n");
	}
}

int
charsperframe()
{
	return termspeed / 10 / framespersec();
}

void
getscreensize()
{
	struct winsize ourwin;
	struct sgttyb ourtty;

	if (ioctl (0, TIOCGWINSZ, &ourwin) != -1) {
		LINES = ourwin.ws_row;
		COLS = ourwin.ws_col;
	}

	if (ioctl (0, TIOCGETP, &ourtty) != -1) {
		termspeed = speeds[ourtty.sg_ospeed];
	}
}

void
die (char *why)
{
	fprintf (stderr, "%s", why);
	exit (1);
}

int
outchar (char what)
{
	printf ("%c", what);
	charsleft--;
	charsused++;
	return what;
}

void
initcap()
{
	TERM = getenv ("TERM");
	if (TERM == 0) {
		die ("Your terminal doesn't know what model it is.  Sorry.\n");
	}

	tgetent (tcapent, TERM);

	clearscreen = tgetstr ("cl", &space);
	if (clearscreen == 0) {
		die ("Your silly terminal can't even clear the screen.\n");
	}

	moveto = tgetstr ("cm", &space);
	if (moveto == 0) {
		die ("Your poor ancient terminal can't do absolute cursor addressing.\nGo buy a new one.\n");
	}

	cleareol = tgetstr ("ce", &space);
	if (cleareol == 0) {
		die ("Oh my!  You can't clear to the end of the line!\n");
	}

	initstring = tgetstr ("is", &space);
	if (initstring) {
		tputs (initstring, 1, outchar);
	}

	sane = tgetstr ("sa", &space);
}

void
gotoyx (int y, int x)
{
	char *move = tgoto (moveto, x, y);
	tputs (move, 1, outchar);
}

void
clearscr()
{
	int x;

	tputs (clearscreen, 1, outchar);
	for (x = 0; x < LINES * COLS; x++) {
		onscreen[x] = ' ';
	}
}

void
inittoast()
{
	int x, y;

	toast[0][0] = ":::::::::______";
	toast[0][1] = "____::::/     /|:____";
	toast[0][2] = "\\   \\__/ / / / _/   /";
	toast[0][3] = ":\\__  / / / / /  __/";
	toast[0][4] = "::::\\/_____/ /__/";
	toast[0][5] = ":::::|     |  /";
	toast[0][6] = ":::::|     | /";
	toast[0][7] = ":::::|_____|/";
	toast[1][0] = ":::::::::______";
	toast[1][1] = "::::::::/     /|";
	toast[1][2] = ":::____/ / / / _______";
	toast[1][3] = ":_/   / / / / /     _/";
	toast[1][4] = "/____/_____/ /_____/";
	toast[1][5] = ":::::|     |  /";
	toast[1][6] = ":::::|     | /";
	toast[1][7] = ":::::|_____|/";
	toast[2][0] = ":::::::::______";
	toast[2][1] = "::::::::/     /|";
	toast[2][2] = "::::___/ / / / ___";
	toast[2][3] = "::_/  / / / / /   \\_";
	toast[2][4] = ":/   /_____/ /___   \\";
	toast[2][5] = "/___/|     |  /::\\___\\";
	toast[2][6] = ":::::|     | /";
	toast[2][7] = ":::::|_____|/";
	toast[3][0] = ":::::::::______";
	toast[3][1] = "::::::::/     /|";
	toast[3][2] = "::::::_/ / / / _";
	toast[3][3] = ":::::// / / / / \\";
	toast[3][4] = ":::://_____/ /   |";
	toast[3][5] = ":::| |     | \\   |";
	toast[3][6] = ":::| |     | /\\  |";
	toast[3][7] = ":::|/|_____|/::\\_|";

	seq[0] = 0;
	seq[1] = 1;
	seq[2] = 2;
	seq[3] = 3;
	seq[4] = 2;
	seq[5] = 0;

	for (x = 0; x < MAXTOASTERS; x++) {
		toasters[x].x = random() % COLS;
		toasters[x].y = random() % LINES;
		toasters[x].frame = random() % FRAMES;
	}

	widest = 0;
	for (x = 0; x < 4; x++) {
		for (y = 0; y < PICLINES; y++) {
			if (strlen (toast[x][y]) > widest) widest = strlen (toast[x][y]);
		}
	}
}

void
clearvscr()
{
	int x;

	for (x = 0; x < LINES * COLS; x++) {
		wantscreen[x] = ' ';
	}
}

void
drawchar (int line, int col, char what)
{
	if (what == ':') return;

	if (line >= 0 && line < LINES && col >= 0 && col < COLS) {
		wantscreen[col * LINES + line] = what;
	}
}

void
drawstring (int line, int col, char *s)
{
	while (*s) {
		drawchar (line, col, *s);
		s++, col++;
		fflush (stdout);
	}
}

void
drawtoaster (int which)
{
	int x;

	for (x = 0; x < PICLINES; x++) {
		drawstring (toasters[which].y - PICLINES + x, toasters[which].x, toast[seq[toasters[which].frame]][x]);
	}
}

int
needfix (int x, int y)
{
	if (x >= COLS || y >= LINES || x < 0 || y < 0) return 0;

	return onscreen[x * LINES + y] != wantscreen[x * LINES + y];
}

int
allspaces (int x, int y) {
	while (x < COLS) {
		if (wantscreen[x * LINES + y] != ' ') return 0;
		x++;
	}
	return 1;
}

void
spacey (int x, int y)
{
	tputs (cleareol, 1, outchar);
	while (x < COLS) {
		onscreen[x * LINES + y] = ' ';
		x++;
	}
}

void
fixscreen (int x, int y)
{
	gotoyx (y, x);
	while (x < COLS && (needfix (x,y) || needfix (x+1, y) || needfix (x+2, y) || needfix (x+3, y))) {
		if (allspaces(x, y)) spacey (x, y);
		else {
			/*outchar ('_');*/
			outchar (wantscreen[x * LINES + y]);
			onscreen[x * LINES + y] = wantscreen[x * LINES + y];
			x++;
		}
	}
}

void
updatescr ()
{
	int x, y;
	static int lasttop = 0;

	charsleft = charsperframe();
	charsused = 0;

	/*tputs (clearscreen, 1, outchar);*/

	for (y = LINES - 1; y >= 0; y--) {
		for (x = 0; x < COLS; x++) {

			if (onscreen[x * LINES + y] != wantscreen[x * LINES + y]) {
				fixscreen (x, y);
			}
		}
	}
}

int
main (int argc, char **argv)
{
	int x;
	int frame = 0;
	static int odd = 0;
	int nomove = 0;

	srandom (time(0));
	getscreensize();

	while (argc > 1) {
		if (argv[1][0] == '-') {
			if (isdigit (argv[1][1])) {
				termspeed = atoi (argv[1] + 1);
			} else if (argv[1][1] == 'c') {
				nomove = 1;
			} else {
				die ("Usage: toast [-c] [-speed] [toasters]\n");
			}
		} else {
			if (isdigit (argv[1][0])) {
				numtoasters = atoi (argv[1]);
				if (numtoasters > MAXTOASTERS) {
					fprintf (stderr, "toast: only %d toasters possible\n", MAXTOASTERS);
					exit (1);
				}
			} else {
				die ("Usage: toast [-c] [-speed] [toasters]\n");
			}
		}
		argc--, argv++;
	}

	makescreen();
	inittoast();
	initcap();
	clearscr();
	while (1) {
		odd = (odd + 1) % 1;

		clearvscr();
		for (x = 0; x < numtoasters; x++) {
			toasters[x].frame++;
			if (toasters[x].frame >= FRAMES) toasters[x].frame = 0;

			if (odd == 0 && nomove == 0) {
				toasters[x].x -= 1;
				toasters[x].y += 1;
			}

			if (toasters[x].y > LINES + PICLINES) {
				toasters[x].y = 0;
				toasters[x].x = random() % COLS;
				toasters[x].frame = random() % FRAMES;
			}

			if (toasters[x].x < 0 - widest) {
				toasters[x].x = COLS;
			}

			drawtoaster (x);
		}
		updatescr();
		fflush (stdout);
		usleep ((1000 * charsused / (termspeed / 10)) * 1000);
		/* usleep (1000000 / framespersec()); */
	}

	tputs (sane, 1, outchar);
}
