/* macterm.c

   attempt to emulate the stuff that a minimal termcappish thing
   will need in a mac window */
   
   
/* this REPLACES term.c and termlib.c -- do NOT link them in when compiling vim
   or bad things (like link errors) will happen.
   
   the necessary bits that aren't mac-specific from those files have been copied
   here, but the mac's weird enough that doing a traditional termcap doesn't
   make a whole lot of sense.
*/
   
#include "vim.h"
#include "globals.h"
#include "param.h"
#include "proto.h"
#include <MacHeaders>
#include <QuickDraw.h>

extern WindowPtr ourWindow;
extern int charwid;
extern int charhigh;
extern int charascent;

extern void setfont (char *);

Tcarr term_strings;

int cursorvis = 1;
int cursorline = 0;
int cursorcol = 0;
int areinverse = 0;
int aremonospaced = 1;

int scrolltop = 0;
int scrollbot = 25;

int canbeep = 1;

/* set_term: set the terminal type to termname[].  But since we're in a Mac
   window, it's going to be about the same whatever the "terminal". */
 
/* and yeah, these should all be #defined instead of using string constants....
*/

void set_term (char *termname) {
	term_strings.t_name = (char_u *)termname;
	term_strings.t_el = (char_u *)"\004";  /* clear to end of line */
	term_strings.t_il = (char_u *)"\005";  /* insert line at cursor */
	term_strings.t_cil = (char_u *)0;
	term_strings.t_dl = (char_u *)"\006";  /* delete line at cursor */
	term_strings.t_cdl = (char_u *)0;
	term_strings.t_cs = (char_u *)"yes yes yes";  /* we have scroll regions, dammit! */
	term_strings.t_ed = (char_u *)"\001";  /* clear screen */
	term_strings.t_ci = (char_u *)"\002";  /* cursor invisible */
	term_strings.t_cv = (char_u *)"\003";  /* cursor visible */
	term_strings.t_cvv = 0;
	term_strings.t_tp = (char_u *)"\017"; /* plain */
	term_strings.t_ti = (char_u *)"\016"; /* reverse */
	term_strings.t_tb = (char_u *)"\016";  /* bold */
	term_strings.t_se = (char_u *)"\017";  /* normal */
	term_strings.t_so = (char_u *)"\016";  /* inverse */
	term_strings.t_cm = (char_u *)"move it!";
	term_strings.t_sr = 0;
	term_strings.t_cri = 0;
	term_strings.t_vb = (char_u *)"\020";  /* visual bell */
	term_strings.t_ks = (char_u *)0;
	term_strings.t_ke = (char_u *)0;
	term_strings.t_ts = (char_u *)0;
	term_strings.t_te = (char_u *)0;
	
	term_strings.t_ku = (char_u*)"\036";  /* up arrow */
	term_strings.t_kd = (char_u*)"\037";  /* down arrow */
	term_strings.t_kl = (char_u*)"\034";  /* left arrow */
	term_strings.t_kr = (char_u*)"\035";  /* right arrow */

	setfont (termname);
}

/* cursor_off: make the cursor invisible next time we need a screen refresh. */

void cursor_off () {
	cursorvis = 0;
}

/* cursor_on: turn the cursor back on. */

void cursor_on () {
	cursorvis = 1;
}

/* breakcheck: see if we've been broken.  maybe this should check for
   cmd-., but doesn't seem crucial.  and i don't much want to deal
   with think c's emulated "signal" library */
   
void breakcheck() {
	;
}

/* chdir should set the current directory to dirname[].  but since the mac
   doesn't have working directories and all the pathnames we receive should
   be full paths, don't bother */
   
void chdir (char *dirname) {
	;
}

RgnHandle aRegion;  /* initialized in mch_windinit, because building and
                       destroying a region every time we try to print
                       a character is kinda expensive. */

/* here is stuff for inserting and deleting lines and scrolling.  this is
   majorly slow if we do it a line at a time, so we wait until something
   *other* than inserting/deleting/scrolling happens, then do it all at
   once, then go back to whatever we're officially doing at the time.
   
   this is really an amazingly major speedup.  unfortunately, it doesn't
   help any when the user's holding down the down-arrow and falling off the
   bottom of the screen, because that's still line-at-a-time because it
   draws each line as it comes on instead of buffering.  sigh.
*/

int ndellines = 0;
int ninslines = 0;
int nscrlines = 0;

int justscrolled = 0;

void fixscrolling() {
	Rect ourRect;
	
	if (ndellines) {  /* delete ndellines lines at cursor */
		ourRect.top = (cursorline + ndellines - 1) * charhigh;
		ourRect.bottom = (scrollbot + 1) * charhigh;
		ourRect.left = ourWindow->portRect.left;
		ourRect.right = ourWindow->portRect.right;
					
		ScrollRect (&ourRect, 0, -ndellines * charhigh, aRegion);
	
		ndellines = 0;
	}
	
	if (ninslines) { /* insert ninslines at cursor */
		ourRect.top = (cursorline) * charhigh;
		ourRect.bottom = (scrollbot + 1) * charhigh;
		ourRect.left = ourWindow->portRect.left;
		ourRect.right = ourWindow->portRect.right;
					
		ScrollRect (&ourRect, 0, charhigh * ninslines, aRegion);
		
		ninslines = 0;
	}
	
	if (nscrlines) { /* scroll the current scroll region nscrlines lines */
		ourRect.top = scrolltop * charhigh;
		ourRect.bottom = (scrollbot + 1) * charhigh;
		ourRect.left = ourWindow->portRect.left;
		ourRect.right = ourWindow->portRect.right;
					
		ScrollRect (&ourRect, 0, -charhigh * nscrlines, aRegion);

		nscrlines = 0;
	}	
}

/* drawone actually draws the character, handles cursor movement, etc.

   what, us do optimization?  we don't care what's on the screen, etc...
   we just draw....
   
   actually, i take that back.  now we do.  every time the cursor moves or
   it's time to read a character, dump out all the stuff that was pending.
   drawone takes care of erasing the space on the screen to the proper color
   (white for normal, black for inverse) but the actual characters are just
   stuffed in pendingdraw until they can be splatted to the screen in a single
   DrawString(), giving quite a bit of speed improvement.
   
   and now we buffer insert/delete/scrolls too, so we're really doing quite
   a bit of optimizing, just in a nontraditional way.  probably not a very
   smart way, come to think of it, but it works....
*/

char pendingdraw[MAX_COLUMNS] = "";
int numpending = 0;
int pendx, pendy;

/* this adds a single character to the set of characters that have theoretically
   been drawn but aren't physically on the screen yet.
*/

void 
bufferchar(char what) {
	if (numpending == 0) {
		pendx = cursorcol;
		pendy = cursorline;
		pendingdraw[0] = what;
		pendingdraw[1] = 0;
		numpending = 1;
	} else {
		pendingdraw[numpending] = what;
		pendingdraw[numpending + 1] = 0;
		numpending++;
	}
}

/* this takes all the pending characters, if there are any, and actually
   spews them into the window.
*/

void 
fixpending() {
	Rect ourrect;
	
	fixscrolling();
	
	if (numpending == 0) return;

	SetPort (ourWindow);
	ourrect.left = pendx * charwid + 2;
	ourrect.top = pendy * charhigh;
	ourrect.right = pendx * charwid + numpending * charwid + 2;
	ourrect.bottom = (pendy + 1) * charhigh;
	
	if (areinverse) {
		PenPat (black);
		PaintRect (&ourrect);
	} else {
		EraseRect (&ourrect);
	}
	
	/* if we're in a monospaced font, just spew the whole thing on the
       screen at once.  otherwise, be nice about it and do it a single
       char at a time (centering in each cell, even!)
    */
    
	if (aremonospaced) {
		MoveTo (pendx * charwid + 3, pendy * charhigh + charascent);
		TextMode (srcXor);
		c2pstr (pendingdraw);
		DrawString ((StringPtr) pendingdraw);
	} else {
		int nowx = pendx * charwid + 3;
		int i;
	
		/* do Bic and Or because we're probably falling outside the rectangle,
		   and it looks nicer just to draw a pixel twice instead of having
		   a doubly-set pixel go blank.
		*/
			
		if (areinverse) {
			TextMode (srcBic);
		} else {
			TextMode (srcOr);
		}

		for (i = 0; i < numpending; i++) {
			MoveTo (pendx * charwid + 3 + i * charwid + charwid / 2 - (CharWidth (pendingdraw[i]) / 2), pendy * charhigh + charascent);	
			DrawChar (pendingdraw[i]);
		}
	}
	numpending = 0;
}

/* draw a single character, handling all the control characters that might be
   in the stream, buffering anything that needs to be buffered.
*/

void 
drawone (char_u what) {	
	SetPort (ourWindow);
	
	if (what >= 32) {
		if (justscrolled && what > 32) {  /* don't force refresh on space or ctrl-char */
			fixpending();
			justscrolled = 0;
		}
		bufferchar (what);
		cursorcol++;
	} else {
		switch (what) {
			case '\001': /* clear screen */
				fixpending();
				EraseRect (&ourWindow->portRect);
				cursorcol = 0;
				cursorline = 0;
			break;
			
			case '\002': /* cursor invisible */
				cursor_off();
			break;
			
			case '\003': /* cursor visible */
				cursor_on();
			break;
			
			case '\004': /* clear to end of line */
				{
					Rect ourRect;
					
					ourRect.top = cursorline * charhigh;
					ourRect.bottom = cursorline * charhigh + charhigh;
					ourRect.left = cursorcol * charwid + 2;
					ourRect.right = ourWindow->portRect.right;
					
					EraseRect (&ourRect);
				}
			break;
			
			case '\005': /* insert line at cursor */
				if (ndellines || nscrlines) fixpending();
				ninslines++;
				/*{
					Rect ourRect;
					
					fixpending();
					ourRect.top = cursorline * charhigh;
					ourRect.bottom = (scrollbot + 1) * charhigh;
					ourRect.left = ourWindow->portRect.left;
					ourRect.right = ourWindow->portRect.right;
					
					ScrollRect (&ourRect, 0, charhigh, aRegion);
				}*/
			break;
			
			case '\006': /* delete line at cursor */
				if (ninslines || nscrlines) fixpending();
				ndellines++;
				/*{
					Rect ourRect;
					
					fixpending();
					ourRect.top = (cursorline) * charhigh;
					ourRect.bottom = (scrollbot + 1) * charhigh;
					ourRect.left = ourWindow->portRect.left;
					ourRect.right = ourWindow->portRect.right;
					
					ScrollRect (&ourRect, 0, -charhigh, aRegion);
				}*/
			break;
			
			case '\016': /* inverse video */
				fixpending();
				areinverse = 1;
			break;
			
			case '\017': /* normal video */
				fixpending();
				areinverse = 0;
			break;
			
			case '\020': /* visual bell */
				fixpending();
				InvertRect (&ourWindow->portRect);
				InvertRect (&ourWindow->portRect);
			break;

			case '\n':
				if (justscrolled == 0) fixpending();
				cursorcol = 0;
				cursorline++;
			break;
			
			case '\r':
				if (justscrolled == 0) fixpending();
				cursorcol = 0;
			break;
			
			case '\t':
				fixpending();
				cursorcol = ((cursorcol + 8) % 8) * 8;
			break;
			
			case '\a':
				fixpending();
				if (canbeep) SysBeep(1);
			break;
			
			default:
				fixpending();
				DrawChar (what+64);
				cursorcol++;
			break;
		}
	}

	while (cursorcol >= Columns) {
		fixpending();
		cursorcol -= Columns;
		cursorline++;
	}
	
	while (cursorline > scrollbot) {
		if (ninslines || ndellines) fixpending();
		nscrlines++;
		/*Rect ourRect;

		fixpending();	
		ourRect.top = scrolltop * charhigh;
		ourRect.bottom = (scrollbot + 1) * charhigh;
		ourRect.left = ourWindow->portRect.left;
		ourRect.right = ourWindow->portRect.right;
					
		ScrollRect (&ourRect, 0, -charhigh, aRegion);
		*/		
		cursorline--;
		justscrolled = 1;
	}
}

/* drawtext: do some actual terminal output.  we'll delegate again... */

void drawtext (char_u *text, int len) {
	int i;
	
	for (i = 0; i < len; i++) drawone (text[i]);
}

/* same for outchar.  how many functions to draw a character *are* there? */

void outchar (char_u what) {
	drawone (what);
}

/* has_wildcard: return true if there's a wildcard in name[] that needs expanded;
   false otherwise.  since we're wimps and not going to do any globbing, just
   act like there's nothing there to expand. */
   
int has_wildcard (char *name) {
	return 0;	
}

/* mktemp: return a temporary filename based on pattern[].  Since we're idiots
   and living dangerously, I'll leave this without doing anything sensible. */
   
char *mktemp (char *pattern) {
	return pattern;
}

/* settmode: set the terminal mode to rawness.  since we're doing everything
   ourselves, don't do anything... */
   
void settmode (int rawness) {
	;
}

/* starttermcap, stoptermcap -- start up and shut down the terminal.  again,
   there's not much we can do.... */
   
void starttermcap() {
	;
}

void stoptermcap() {
	;
}

/* set_scroll_region sets the scroll region; reset_scroll_region resets it.
   these are supposed to send terminal strings but since we're doing the
   terminal stuff inside, we just set the variables */
   
/*
 * set scrolling region for window 'wp'
 */
	void
scroll_region_set(wp)
	WIN		*wp;
{
	fixpending();
	
	scrollbot = wp->w_winpos + wp->w_height - 1;
	scrolltop = wp->w_winpos;
}

/*
 * reset scrolling region to the whole screen
 */
	void
scroll_region_reset()
{
	fixpending();
	
	scrollbot = Rows - 1;
	scrolltop = 0;
}

/* windgoto moves the virtual cursor to the following line and column. */

void windgoto (int line, int col) {
	if (justscrolled == 0) fixpending();
	cursorline = line;
	cursorcol = col;
}

/* tgoto is supposed to return a string to move the cursor to col, line,
   but we can probably get by with moving the cursor ourselves and then
   sending a null string that represents the movement. */
   
char *tgoto (char *cm, int col, int line) {
	return "";
}

/* ttest should be checking what our terminal capabilities are.
   we know what they are, but check anyway...*/
   
	void
ttest(pairs)
	int	pairs;
{
	char buf[70];
	char *s = "terminal capability %s required.\n";
	char *t = NULL;

  /* hard requirements */
	if (!T_ED || !*T_ED)	/* erase display */
		t = "cl";
	if (!T_CM || !*T_CM)	/* cursor motion */
		t = "cm";

	if (t)
    {
    	sprintf(buf, s, t);
    	EMSG(buf);
    }

/*
 * if "cs" defined, use a scroll region, it's faster.
 */
	if (T_CS && *T_CS != NUL)
		{
			scroll_region = TRUE;
			scroll_region_reset();
		}
	else
		scroll_region = FALSE;

	if (pairs)
	{
	  /* optional pairs */
			/* TP goes to normal mode for TI (invert) and TB (bold) */
		if ((!T_TP || !*T_TP))
			T_TP = T_TI = T_TB = NULL;
		if ((!T_SO || !*T_SO) ^ (!T_SE || !*T_SE))
			T_SO = T_SE = NULL;
			/* T_CV is needed even though T_CI is not defined */
		if ((!T_CV || !*T_CV))
			T_CI = NULL;
			/* if 'mr' or 'me' is not defined use 'so' and 'se' */
		if (T_TP == NULL || *T_TP == NUL)
		{
			T_TP = T_SE;
			T_TI = T_SO;
			T_TB = T_SO;
		}
			/* if 'so' or 'se' is not defined use 'mr' and 'me' */
		if (T_SO == NULL || *T_SO == NUL)
		{
			T_SE = T_TP;
			if (T_TI == NULL)
				T_SO = T_TB;
			else
				T_SO = T_TI;
		}
	}
}

/* WaitForChar waits the right number of milliseconds before getting annoyed
   that there are no characters pending... */
   
int WaitForChar (long millisec) {
	long now = TickCount();
	long ticks = millisec * 60 / 1000;
	EventRecord myEvent;
	
	while (TickCount() < now + ticks) {
		if (EventAvail (keyDownMask | autoKeyMask, &myEvent)) return 1;
	}
	return 0;
}

void termcapinit (char *term) {
	set_term(term);
	ttest (TRUE);
}

/*********************************************************************

   stuff stolen from the real term.c
   from now on.
   
 *********************************************************************/

/* from term.c */

/*
 * the number of calls to mch_write is reduced by using the buffer "outbuf"
 */
 
#define BSIZE 2048
static char_u			outbuf[BSIZE];
static int				bpos = 0;		/* number of chars in outbuf */

/*
 * flushbuf(): flush the output buffer
 
 this from term.c
 
 */

void flushbuf()
{
	if (bpos != 0)
	{
		mch_write(outbuf, bpos);
		bpos = 0;
	}
}

/*
 * Set cursor to current position.
 * Should be optimized for minimal terminal output.
 */

	void
setcursor()
{
	if (!RedrawingDisabled)
		windgoto(curwin->w_winpos + curwin->w_row, curwin->w_col);
}


/*

   this from term.c -- i didn't write it.
 
 * inchar() - get one character from
 *		1. a scriptfile
 *		2. the keyboard
 *
 *  As much characters as we can get (upto 'maxlen') are put in buf and
 *  NUL terminated (buffer length must be 'maxlen' + 1).
 *
 *	If we got an interrupt all input is read until none is available.
 *
 *  If time == 0  there is no waiting for the char.
 *  If time == n  we wait for n msec for a character to arrive.
 *  If time == -1 we wait forever for a character to arrive.
 *
 *  Return the number of obtained characters.
 */

	int
inchar(buf, maxlen, time)
	char_u	*buf;
	int		maxlen;
	int		time;						/* milli seconds */
{
	int				len;
	int				retesc = FALSE;		/* return ESC with gotint */
	register int 	c;
	register int	i;

	if (time == -1 || time > 100)	/* flush output before waiting */
	{
		cursor_on();
		flushbuf();
	}
	did_outofmem_msg = FALSE;	/* display out of memory message (again) */

/*
 * first try script file
 *	If interrupted: Stop reading script files.
 */
retry:
	if (scriptin[curscript] != NULL)
	{
		if (got_int || (c = getc(scriptin[curscript])) < 0)	/* reached EOF */
		{
				/* when reading script file is interrupted, return an ESC to
									get back to normal mode */
			if (got_int)
				retesc = TRUE;
			fclose(scriptin[curscript]);
			scriptin[curscript] = NULL;
			if (curscript > 0)
				--curscript;
			goto retry;		/* may read other script if this one was nested */
		}
		if (c == 0)
			c = K_ZERO;		/* replace ^@ with special code */
		*buf++ = c;
		*buf = NUL;
		return 1;
	}

/*
 * If we got an interrupt, skip all previously typed characters and
 * return TRUE if quit reading script file.
 */
	if (got_int)			/* skip typed characters */
	{
		while (GetChars(buf, maxlen, T_PEEK))
			;
		return retesc;
	}
	len = GetChars(buf, maxlen, time);

	for (i = len; --i >= 0; ++buf)
		if (*buf == 0)
			*(char_u *)buf = K_ZERO;		/* replace ^@ with special code */
	*buf = NUL;								/* add trailing NUL */
	return len;
}

/*
 * a never-padding outstr.
 * use this whenever you don't want to run the string through tputs.
 * tputs above is harmless, but tputs from the termcap library 
 * is likely to strip off leading digits, that it mistakes for padding
 * information. (jw)
 */
	void
outstrn(s)
	char_u *s;
{
	if (bpos > BSIZE - 20)		/* avoid terminal strings being split up */
		flushbuf();
	while (*s)
		outchar(*s++);
}

/*
 * outstr(s): put a string character at a time into the output buffer.
 * If TERMCAP is defined use the termcap parser. (jw)
 */
	void
outstr(s)
	register char_u			 *s;
{
	if (bpos > BSIZE - 20)		/* avoid terminal strings being split up */
		flushbuf();
	if (s)
		while (*s)
			outchar(*s++);
}

	void
check_winsize()
{
	if (Columns < MIN_COLUMNS)
		Columns = MIN_COLUMNS;
	else if (Columns > MAX_COLUMNS)
		Columns = MAX_COLUMNS;
	if (Rows < MIN_ROWS + 1)	/* need room for one window and command line */
		Rows = MIN_ROWS + 1;
	screen_new_rows();			/* may need to update window sizes */
}

/*
 * set window size
 * If 'mustset' is TRUE, we must set Rows and Columns, do not get real
 * window size (this is used for the :win command).
 * If 'mustset' is FALSE, we may try to get the real window size and if
 * it fails use 'width' and 'height'.
 */
	void
set_winsize(width, height, mustset)
	int		width, height;
	int		mustset;
{
	register int 		tmp;

	if (width < 0 || height < 0)	/* just checking... */
		return;

	if (State == HITRETURN || State == SETWSIZE)	/* postpone the resizing */
	{
		State = SETWSIZE;
		return;
	}
	screenclear();
	if (mustset || mch_get_winsize() == FAIL)
	{
		Rows = height;
		Columns = width;
		scroll_region_reset();
		check_winsize();		/* always check, to get p_scroll right */
		mch_set_winsize();
	}
	else
		check_winsize();		/* always check, to get p_scroll right */
	if (State == HELP)
		(void)redrawhelp();
	else if (!starting)
	{
		tmp = RedrawingDisabled;
		RedrawingDisabled = FALSE;
		comp_Botline_all();
		updateScreen(CURSUPD);
		RedrawingDisabled = tmp;
		if (State == CMDLINE)
			redrawcmdline();
		else
			setcursor();
	}
	flushbuf();
}

