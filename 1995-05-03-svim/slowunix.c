/* slowunix.c

   stuff to make using vim on a slow terminal less painful.
   revised version, works on loads more terminals.

   Eric Fischer, etaoin@uchicago.edu
*/

#include <stdio.h>
#include <stdlib.h>
#include <sgtty.h>

#include "vim.h"
#include "unix.h"
#include "proto.h"

#include "terms.h"

/* these are #defined to point to our functions rather than the
   real ones.  we need the real ones to do real output.
*/

#undef tgoto
#undef tgetent
#undef tputs
#undef tgetstr

char *tgetstr (char *what, char **buf);
char *tgetent (char *buf, char *name);
char *tgoto (char *cm, int col, int row);
void tputs (char *cp, int count, int (*func)(char foo));
void setphysscroll (int, int);

/* arrays to hold real and virtual screens */

char *wantscreen = 0;
char *onscreen = 0;
char *wantattr = 0;
char *onattr = 0;

/* size of the screen */

int scrrows = 0;
int scrcols = 0;

/* cursor position */

int virtx = 0;
int virty = 0;
int physx = 0;
int physy = 0;
int oldvirtx = 0, oldvirty = 0;  /* fake, inverse video, cursor */

/* current inverseness */

int virtattr = 0;
int physattr = 0;

/* termcap strings */

char *clearscreencmd = 0;
char *setscrollcmd = 0;
char *scrolldowncmd = 0;
char *standoutcmd = 0;
char *standendcmd = 0;
char *cleareolcmd = 0;
char *insertcmd = 0;
char *deletecmd = 0;

char *movecurscmd = 0;
char *homecmd = 0;
char *upcmd = 0;
char *downcmd = 0;
char *leftcmd = 0;

/* scroll regions */

int virtscrolltop = 0;
int virtscrollbot = 0;
int physscrolltop = 0;
int physscrollbot = 0;

/* the tty's speed */

int outspeed = 0;

/* tty size */

extern int Rows;
extern int Columns;

/* flow control */

int charsleft = 0;
int stillpending = 0;


extern int scroll_region;


/* calculate where in the array corresponds to some screen coordinates.
   should be inline?

   if out of bounds, just return 0, even though it'll toast the top left
   of the screen.
*/

static int
loc (int y, int x)
{
	if (y < 0 || y >= scrrows || x < 0 || x >= scrcols) {
		/* fprintf (stderr, "out of bounds: %d %d", y, x); */
		return scrrows * scrcols - 1;
	}

	return y * scrcols + x;
}

/* used by termcap routines to actually do output.  no idea what
   tputs() wants it to return, but i don't think it really cares.
*/

static int
putfunc (char what)
{
	printf ("%c", what);
	charsleft--;

	return what;  /* is this right? */
}

/* if we don't have standout mode, try to do something vaguely
   highlightish.  with luck, we'll have a reasonable terminal and
   it won't matter...
*/

static int
putfunkyfunc (char what)
{
	if (islower (what)) what -= 32;
	if (what == ' ') what = '_';

	putfunc (what);
	return what;
}

void
reallyclearscreen()
{
	tputs (clearscreencmd, 1, putfunc);
	physx = 0;
	physy = 0;
}

/* fill an array with spaces */

static void
blank (char *where, int len)
{
	while (len) {
		*where = ' ';
		where++, len--;
	}
}

/* zero an array of the specified len */

static void
zero (char *where, int len)
{
	while (len) {
		*where = 0;
		where++, len--;
	}
}

/* called when the screen gets resized, or when we start up.
   deallocate any previous memory used to save screens and make
   new ones of the specified size.
*/

void
newscreensize (int rows, int cols)
{
	scrrows = rows;
	scrcols = cols;

	if (wantscreen) free (wantscreen);
	if (onscreen) free (onscreen);
	if (wantattr) free (wantattr);
	if (onattr) free (onattr);

	wantscreen = malloc (rows * cols * sizeof (char));
	onscreen = malloc (rows * cols * sizeof (char));
	wantattr = malloc (rows * cols * sizeof (char));
	onattr = malloc (rows * cols * sizeof (char));

	if (wantscreen == 0 || onscreen == 0 || wantattr == 0 || onattr == 0) {
		fprintf (stderr, "Vim: malloc failure!\n");
		exit (1);
	}

	blank (wantscreen, rows * cols);
	blank (onscreen, rows * cols);
	zero (wantattr, rows * cols);
	zero (onattr, rows * cols);

	if (clearscreencmd) {
		reallyclearscreen();
	} else {
		virtx = 0;
		virty = 0;
		physx = -1;
		physy = -1;

		zero (onscreen, rows * cols);
		blank (onattr, rows * cols);
	}

	physscrolltop = -1;
	physscrollbot = -1;

/*	physscrolltop = 0;
	physscrollbot = Rows - 1;
*/
	virtscrolltop = 0;
	virtscrollbot = Rows - 1;
}

/* return the speed of our terminal.  if BPS is set, use that;
   if not, use ioctl.  if ioctl fails, just act like it's 9600.
*/

static int speeds[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400,
	4800, 9600, 19200, 38400
};

int
getttyspeed()
{
	char *BPS;
	int speed;
	struct sgttyb get;

	if ((BPS = getenv ("BPS"))) {
		speed = atoi (BPS);
		if (BPS) return speed;
	}

	if (ioctl (0, TIOCGETP, &get) != -1) {
		return speeds[(int) get.sg_ospeed];
	}

	return 9600;
}

void
slowerr (char *why)
{
	fprintf (stderr, "vim: Your terminal can't %s.  Sorry\n", why);
	exit (1);
}

/* get all the termcap commands we need.

   complain if we can't move the cursor, because that's just *too* dumb
   a terminal to deal with...
*/

void
reallyinittcap()
{
	char *TERM = getenv ("TERM");
	static char buf1 [2048], buf2[2048], *here = buf2;

	if (TERM == 0) {
		fprintf (stderr, "vim: TERM must be set to terminal type\n");
		exit (1);
	}

	tgetent (buf1, TERM);

	clearscreencmd = tgetstr ("cl", &here); 
	movecurscmd = tgetstr ("cm", &here); 
	setscrollcmd = tgetstr ("cs", &here); 
	scrolldowncmd = tgetstr ("sr", &here); 
	standoutcmd = tgetstr ("so", &here);
	standendcmd = tgetstr ("se", &here);
	cleareolcmd = tgetstr ("ce", &here);
	insertcmd = tgetstr ("al", &here);
	deletecmd = tgetstr ("dl", &here);

	upcmd = tgetstr ("up", &here);
	downcmd = tgetstr ("do", &here);
	leftcmd = tgetstr ("le", &here);
	homecmd = tgetstr ("ho", &here);

	if (movecurscmd == 0
	&& (homecmd == 0 || leftcmd == 0 /* || rightcmd == 0 */
	||  downcmd == 0 || upcmd == 0)) slowerr ("move the cursor");

	outspeed = getttyspeed();

	newscreensize (Rows, Columns);
}

/* called from term.c.  move the virtual cursor somewhere. */

void
windgoto (int row, int col)
{
	flushbuf();
	if (row >= 0 && row < scrrows && col >=0 && col < scrcols) {
		virtx = col;
		virty = row;
	}
}

/* set the actual terminal's attribute to whatever.  if it's
   already that, don't do anything.
*/

void
attrto (int what)
{
	if (physattr == what) return;

	physattr = what;
	if (what) {
		if (standoutcmd) tputs (standoutcmd, 1, putfunc);
	} else {
		if (standendcmd) tputs (standendcmd, 1, putfunc);
	}
}

/* send out a single character from the array.
   used by cursto() in optimizing
*/

void
spewone (int x, int y)
{
	int lyx = loc (y, x);

	if (standoutcmd) {
		attrto  (wantattr  [lyx]);
		putfunc (wantscreen[lyx]);
	} else {
		if (wantattr [lyx]) putfunkyfunc (wantscreen[lyx]);
		else                putfunc      (wantscreen[lyx]);
	}
	onscreen[lyx] = wantscreen[lyx];
	onattr  [lyx] = wantattr  [lyx];
}

/* move the cursor to wherever.  we do a little optimizing --
   if it's already there we don't do anything, and if we can
   get there by backspacing or printing a few chars, we do
   that instead of actually cming.

   if we need to move the cursor manually, do that.
*/

void
cursto (int row, int col)
{
	if (row != physy || col != physx) {
		char *go;

		if (row == physy) {
			if (col == physx - 1) {
				putfunc ('\b');
				physx = col;
				return;
			} else if (col == physx + 1) {
				spewone (physx, physy);
				physx++;
				return;
			} else if (col == physx + 2) {
				spewone (physx, physy);
				spewone (physx + 1, physy);
				physx += 2;
				return;
			}
		}

		if (movecurscmd) {
			go = tgoto (movecurscmd, col, row);
			tputs (go, 1, putfunc);
		} else {
			if (setscrollcmd) {
				setphysscroll (virtscrollbot, virtscrolltop);
			}

			if (physx ==  -1 || physy == -1
			||  physx == 999 || physy == 999) {
				tputs (homecmd, 1, putfunc);
				physx = 0; physy = 0;
			}

			while (row < physy) {
				tputs (upcmd, 1, putfunc);
				physy--;
			}
	
			while (row > physy) {
				tputs (downcmd, 1, putfunc);
				physy++;
			}

			while (col < physx) {
				tputs (leftcmd, 1, putfunc);
				physx--;
			}

			while (col > physx) {
				spewone (physx, physy);
				/* tputs (rightcmd, 1, putfunc); */
				physx++;
			}
		}

		fflush (stdout);
	}

	physy = row;
	physx = col;
}

/* set the virtual scroll region to whatever.  the physical one
   will get set when we actually scroll.

   called from inside term.c
*/

void
setscroll (int bot, int top)
{
	flushbuf();

	virtscrolltop = top;
	virtscrollbot = bot;
}

/* this is what other .c files get when they call tgoto().
   really it should send out a character string that will
   alert spewchar(), but since we're just lazy we hope that
   no one will call it for anything other than an actual
   move and just set the vars ourself.
*/

char *
our_tgoto (char *how, int col, int row)
{
	flushbuf();

	if (col >= 0 && col < scrcols && row >= 0 && row <= scrrows) {
		virtx = col;
		virty = row;
	}

	return how;
}

/* this is supposed to get the terminal size from termcap.
   but since we have ioctls to do it, just forget it.
*/

void
getlinecol()
{
}

/* set physical scroll region.  if it's already what we want,
   do nothing.
*/

void
setphysscroll (int bot, int top)
{
	char *s;

	if (physscrollbot == bot && physscrolltop == top) return;

	physscrollbot = bot;
	physscrolltop = top;

	s = tgoto (setscrollcmd, bot, top);
	tputs (s, 1, putfunc);

	/* I have no idea why we need to bounce the cursor around like
	   this.  In fact, it's not necessary on the NeXT console, at
	   least.  But the Mac with ZTerm doesn't work right without
	   it, so here it is.
	*/

/*
	cursto (top, 0);
	cursto (bot, 0);
*/

	/* Actually, I think this should be sufficient.  We'll see,
	   I guess.
	*/

	physx = -1; physy = -1;
	cursto (virty, virtx); 
	fflush (stdout);
}

/* clear to end of the line.  if it's supported in hardware, do that;
   otherwise, just put spaces in the buffer and do it by hand.
*/

void
docleareol()
{
	int x;

	/* if we have hardware cleareol, use it; otherwise, fake it */

	if (cleareolcmd) {
		cursto (virty, virtx);
		tputs (cleareolcmd, 1, putfunc);

		for (x = virtx; x < scrcols; x++) {
			int lyx = loc (virty, x);

			onscreen[lyx] = ' ';
			wantscreen[lyx] = ' ';

			onattr[lyx] = 0;
			wantattr[lyx] = 0;
		}
	} else {
		for (x = virtx; x < scrcols; x++) {
			int lyx = loc (virty, x);

			wantscreen[lyx] = ' ';
			wantattr[lyx] = 0;
		}

		stillpending = 1;
	}

	fflush (stdout);
}

/* use scroll regions to insert a blank line before 'virty',
   and set the arrays to include the new line and shove everything
   else down.

   if there are no scroll regions, look for hardware line insert/delete
   support; if there's not that, just shove everything and do it
   by hand later.
*/

void
doinsertln()
{
	int x, y;

	/* Zterm can't cope with a one-line scroll region, so erase
	   the line ourself if we're only doing one.
	   Just checked, and NCSA Telnet can't either. 
	*/

	if (virtscrollbot == virty) {
		int oldvirtx;

		oldvirtx = virtx;
		virtx = 0;
		docleareol();
		virtx = oldvirtx;
		return;
	}

	if (scrolldowncmd && setscrollcmd) {
		setphysscroll (virtscrollbot, virty);

		cursto (virty, virtx);
		tputs (scrolldowncmd, 1, putfunc);

		for (y = virtscrollbot - 1; y >= virty; y--) {
			for (x = 0; x < scrcols; x++) {
				int ly1x = loc (y+1, x);
				int lyx = loc (y, x);

				wantscreen[ly1x] = wantscreen[lyx];
				onscreen[ly1x] = onscreen[lyx];

				wantscreen[lyx] = ' ';
				onscreen[lyx] = ' ';

				wantattr[ly1x] = wantattr[lyx];
				onattr[ly1x] = onattr[lyx];

				wantattr[lyx] = 0;
				onattr[lyx] = 0;
			}
		}

	} else if (insertcmd && deletecmd) {
		cursto (virtscrollbot, 0);
		tputs (deletecmd, 1, putfunc);
		cursto (virty, 0);
		tputs (insertcmd, 1, putfunc);

		for (y = virtscrollbot - 1; y >= virty; y--) {
			for (x = 0; x < scrcols; x++) {
				int ly1x = loc (y+1, x);
				int lyx = loc (y, x);

				wantscreen[ly1x] = wantscreen[lyx];
				onscreen[ly1x] = onscreen[lyx];

				wantscreen[lyx] = ' ';
				onscreen[lyx] = ' ';

				wantattr[ly1x] = wantattr[lyx];
				onattr[ly1x] = onattr[lyx];

				wantattr[lyx] = 0;
				onattr[lyx] = 0;
			}
		}
	} else {
		for (y = virtscrollbot - 1; y >= virty; y--) {
			for (x = 0; x < scrcols; x++) {
				int ly1x = loc (y+1, x);
				int lyx = loc (y, x);

				wantscreen[ly1x] = wantscreen[lyx];
				wantscreen[lyx] = ' ';
				wantattr[ly1x] = wantattr[lyx];
				wantattr[lyx] = 0;
			}
		}
	}

	oldvirty++;
	fflush (stdout);
}

/* set the scroll regions up to delete line 'virty',
   and move stuff around in the arrays to reflect that it's gone.
*/

void
dodeleteln()
{
	int x, y;

	/* deleting the bottom line is similarly annoying.
	   odd that in this degenerate case, inserting and
	   deleting a line mean exactly the same thing!
	*/

	if (virtscrollbot == virty) {
		int ovirtx;

		ovirtx = virtx;
		virtx = 0;
		docleareol();
		virtx = ovirtx;
		return;
	}

	if (setscrollcmd) {
		setphysscroll (virtscrollbot, virty);

		physx = -1, physy = -1;

		cursto (virtscrollbot, 0);
		putfunc ('\n');

		for (y = virty + 1; y <= virtscrollbot; y++) {
			for (x = 0; x < scrcols; x++) {
				int lyx = loc (y, x);
				int ly1x = loc (y-1, x);

				wantscreen[ly1x] = wantscreen[lyx];
				onscreen[ly1x] = onscreen[lyx];

				wantscreen[lyx] = ' ';
				onscreen[lyx] = ' ';

				wantattr[ly1x] = wantattr[lyx];
				onattr[ly1x] = onattr[lyx];

				wantattr[lyx] = 0;
				onattr[lyx] = 0;
			}
		}

	} else {
		if (insertcmd && deletecmd) {
			cursto (virty, 0);
			tputs (deletecmd, 1, putfunc);
			cursto (virtscrollbot, 0);
			tputs (insertcmd, 1, putfunc);

			for (y = virty + 1; y <= virtscrollbot; y++) {
				for (x = 0; x < scrcols; x++) {
					int lyx = loc (y, x);
					int ly1x = loc (y-1, x);

					wantscreen[ly1x] = wantscreen[lyx];
					onscreen[ly1x] = onscreen[lyx];

					wantscreen[lyx] = ' ';
					onscreen[lyx] = ' ';

					wantattr[ly1x] = wantattr[lyx];
					onattr[ly1x] = onattr[lyx];

					wantattr[lyx] = 0;
					onattr[lyx] = 0;
				}
			}
		} else {
			for (y = virty + 1; y <= virtscrollbot; y++) {
				for (x = 0; x < scrcols; x++) {
					int lyx = loc (y, x);
					int ly1x = loc (y-1, x);

					wantscreen[ly1x] = wantscreen[lyx];
					wantscreen[lyx] = ' ';
					wantattr[ly1x] = wantattr[lyx];
					wantattr[lyx] = 0;
				}
			}

		}
	}

	oldvirty--;

	fflush (stdout);
}

/* clear the screen for real, and blank out both physical and
   wanted screen arrays.  perhaps it'd be better not to actually
   clear the screen but just set wantscreen[] to blanks, so we
   could use whatever was there already.

   no, actually, that usually sucks.  we do that if the hardware
   doesn't support clearing the screen, though!
*/

void
doclearscr()
{
	int y, x;

	if (clearscreencmd) {
		reallyclearscreen();

		virtx = 0; virty = 0;

		for (x = 0; x < scrcols; x++) {
			for (y = 0; y < scrrows; y++) {
				int lyx = loc (y, x);

				onscreen[lyx] = ' ';
				wantscreen[lyx] = ' ';

				onattr [lyx] = 0;
				wantattr [lyx] = 0;
			}
		}
	} else {
		virtx = 0; virty = 0;

		for (x = 0; x < scrcols; x++) {
			for (y = 0; y < scrrows; y++) {
				int lyx = loc (y, x);

				wantscreen[lyx] = ' ';
				onscreen[lyx] = 0;
				wantattr [lyx] = 0;
				onattr [lyx] = 32;
			}
		}
	}
}

/* turn inverse video on... */

void
dostandout()
{
	virtattr = 1;
}

/* and off.... */

void
dostandend()
{
	virtattr = 0;
}

void
beepbeep()
{
	putfunc (7);
}

/* move everything in the scroll region up a line.

   spewchar() handles actually setting the region and
   printing the \n.
*/

void
doscrollup()
{
	int x, y;

	for (y = virtscrolltop + 1; y <= virtscrollbot; y++) {
		for (x = 0; x < scrcols; x++) {
			int ly1x = loc (y-1, x);
			int lyx = loc (y, x);

			wantscreen[ly1x] = wantscreen[lyx];
			onscreen[ly1x] = onscreen[lyx];
			wantattr[ly1x] = wantattr[lyx];
			onattr[ly1x] = onattr[lyx];

			wantscreen[lyx] = ' ';
			onscreen[lyx] = ' ';
			wantattr[lyx] = 0;
			onattr[lyx] = 0;
		}
	}

	oldvirty--;
}

/* terminal emulation!

   perform an action for each of the special characters,
   or if it's nothing special at all stuff it in the
   screen array.

   also handle part of the scrolling that takes place
   when we fall off the bottom of the screen.
*/

void
spewchar (char what)
{
	if        (what == CCLEAREOL) {
		docleareol();
	} else if (what == CINSERTLN) {
		doinsertln();
	} else if (what == CDELETELN) {
		dodeleteln();
	} else if (what == CCLEARSCR) {
		doclearscr();
	} else if (what == CSTANDOUT) {
		dostandout();
	} else if (what == CSTANDEND) {
		dostandend();
	} else if (what == '\r') {
		virtx = 0;
	} else if (what == '\n') {
		virty++;
	} else if (what == '\t') {
		virtx = ((virtx + 8) % 8) * 8;
	} else if (what == 7) {
		beepbeep();
	} else if (what >= ' ') {
		stillpending = 2;
		wantscreen[loc (virty, virtx)] = what;
		wantattr  [loc (virty, virtx)] = virtattr;
		virtx++;
	} else {
		fprintf (stderr, "%d\n", what);
		exit (1);
	}

	if (virtx >= scrcols) {
		virty++;
		virtx -= scrcols;
	}

	if (virty > virtscrollbot) {
		while (virty > virtscrollbot) {
			int savex = virtx, savey = virty;

			virtx = 0;
			virty = virtscrolltop;
			dodeleteln();
			virtx = savex;
			virty = savey;

			virty--;
		}
	}
}

/* update at least one character on the screen, and possibly
   a few more if they need updating and are in the neighborhood.
*/

void
fixchar (int y, int x)
{
	while (1) {
		int ll = loc (y, x);

		cursto (y, x);

		if (standoutcmd) {
			attrto (wantattr [ll]);
			putfunc (wantscreen[ll]);
		} else {
			if (wantattr[ll]) putfunkyfunc (wantscreen[ll]);
			else              putfunc      (wantscreen[ll]);
		}

		onscreen[ll] = wantscreen[ll];
		onattr  [ll] = wantattr  [ll];

		x++;
		physx++;
		if (x >= scrcols) return;

		if (wantscreen[loc (y,x)] == onscreen[loc (y,x)] &&
		    wantscreen[loc (y,x+1)] == onscreen[loc (y,x+1)] &&
		    wantscreen[loc (y,x+2)] == onscreen[loc (y,x+2)] &&

		    wantattr[loc (y,x)] == onattr[loc (y,x)] &&
		    wantattr[loc (y,x+1)] == onattr[loc (y,x+1)] &&
		    wantattr[loc (y,x+2)] == onattr[loc (y,x+2)])
		{
			return;
		}

		if (charsleft <= 0) return;
	}
}

long
charspersec()
{
	if (outspeed == 38400) {
		return 200000;
	}

	return outspeed / 12;
}

#define UNKNOWN 3
#define EXTRA 7

void
fixscreen (int len)
{
	int y, x;
	int temp;
	int ll = loc (virty, virtx);
	static int boring = UNKNOWN;
	int oldphysx = physx, oldphysy = physy;

	if (boring == UNKNOWN) {
		boring = (getenv ("BORING") != 0);
	}

	if (charsleft >= 0) charsleft = 0;
	charsleft += len * charspersec() / 1000;

	if (stillpending == 0) return;

	/* if the cursor is away, draw a fake one. */

	if (oldphysx != virtx || oldphysy != virty) {
		wantattr[ll] = !wantattr[ll];
	}

	if (charsleft > 0) {
		/* update the screen around the cursor */

		temp = virtx - 5;
		if (temp < 0) temp = 0;

		for (x = temp; x < temp + 10 && x < scrcols; x++) {
			int lol = loc (virty, x);
		    
			if (onscreen[lol] != wantscreen[lol] ||
			    onattr  [lol] != wantattr  [lol]) {
				fixchar (virty, x);
			}
		}

		/* get rid of last time's fake cursor, if it's still there */

		temp = oldvirtx - 5;
		if (temp < 0) temp = 0;

		for (x = temp; x < temp + 10 && x < scrcols; x++) {
			int lol = loc (oldvirty, x);
		    
			if (onscreen[lol] != wantscreen[lol] ||
			    onattr  [lol] != wantattr  [lol]) {
				fixchar (oldvirty, x);
			}
		}

		/* remember that we left a cursor here */

		oldvirtx = virtx;
		oldvirty = virty;
	}
	
	/* if boring, rewrite a hunk of the screen in linear order.
	   if not boring, do it both top-to-bottom and vice versa.
	*/

	if (boring) {
		for (y = 0; y < scrrows; y++) {
			for (x = 0; x < scrcols; x++) {
				int lyx = loc (y, x);

				if (onscreen[lyx] != wantscreen[lyx] ||
				    onattr  [lyx] != wantattr  [lyx]) {
					if (charsleft > 0) fixchar (y, x);
				}
			}
		}
	} else {
		for (y = 0; y < scrrows; y++) {
			int yy = scrrows - 1 - y;

			for (x = 0; x < scrcols; x++) {
				int lyx = loc (y, x);

				if (onscreen[lyx] != wantscreen[lyx] ||
				    onattr  [lyx] != wantattr  [lyx]) {
					if (charsleft > 0) fixchar (y, x);
				}
			}

			for (x = 0; x < scrcols; x++) {
				int lyx = loc (yy, x);

				if (onscreen[lyx] != wantscreen[lyx] ||
				    onattr  [lyx] != wantattr  [lyx]) {
					if (charsleft > 0) fixchar (yy, x);
				}
			}
		}
	}

	/* erase the fake cursor (from memory) */

	if (oldphysx != virtx || oldphysy != virty) {
		wantattr[ll] = !wantattr[ll];
	}

	/* if charsleft, then we're done!  update the area
	   around the cursor again (to get rid of the fake one)
	   then move the real one there.
	*/

	if (charsleft > 0) {
		stillpending = 0;

		temp = virtx - 5;
		if (temp < 0) temp = 0;

		for (x = temp; x < temp + 10 && x < scrcols; x++) {
			int lol = loc (virty, x);
		    
			if (onscreen[lol] != wantscreen[lol] ||
			    onattr  [lol] != wantattr  [lol]) {
				fixchar (virty, x);
			}
		}

		cursto (virty, virtx);
	}

	if (setscrollcmd && (charsleft > 0)) {
		setphysscroll (virtscrollbot, virtscrolltop);
	}

	fflush (stdout);
}
