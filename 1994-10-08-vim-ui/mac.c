/* vi:ts=4:sw=4
 *
 * VIM - Vi IMproved
 *
 * Read the file "credits.txt" for a list of people who contributed.
 * Read the file "uganda.txt" for copying and usage conditions.
 */

/*
 * mac.c
 *
 * make our mac happy, we hope.
 * this is vaguely based on amiga.c (the function names are, at least)
 * but the mac interface is so weirdly different from the usual systems
 * vim is used to running on that there's much weirdness here.
 *
 * by Eric Fischer, etaoin@uchicago.edu
 *
 */

#include "vim.h"
#include "globals.h"
#include "proto.h"
#include "param.h"

#include <MacHeaders>
#include <QuickDraw.h>
#include <Memory.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <OSUtils.h>
#include <Files.h>
#include <StandardFile.h>
#include <SegLoad.h>
#include <Fonts.h>
#include <BDC.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>

#include <fcntl.h>

#undef TRUE 			/* will be redefined by exec/types.h */
#undef FALSE

/*
 * At this point TRUE and FALSE are defined as 1L and 0L, but we want 1 and 0.
 */
#undef TRUE
#define TRUE (1)
#undef FALSE
#define FALSE (0)

int firstone = 1;  /* are we opening the first file? if so, use :vi instead of :new */

int charwid = 6;         /* these are monaco-9 values; will get changed */
int charhigh = 11;       /* as soon as the font changes */
int charascent = 9;

int shouldrows, shouldcols;  /* window size we want.  does this get used??? */

extern int canbeep;  /* are we allowed to beep? */

extern void drawone (char);  /* draw a single character */
extern int cursorline;       /* where the cursor is right now */
extern int cursorcol;
extern int aremonospaced;  /* so we know whether we can draw quickly or not */

/* this holds whether the cursor is visible or not.  however, since the only
   time the cursor is hidden is during screen refreshes, when we're not drawing
   cursors anyway, there's not much point.  oh well.
*/

extern int cursorvis;   /* should we draw the cursor?  (ignored) */

extern RgnHandle aRegion;  /* to be initialized by the window init, and used
                              to scroll the screen in macterm.c */

extern long AvailMem();
void mch_settitle (char_u *, char_u *);
void drawCursor (void);
void mch_set_winsize (void);

#define MAXTYPEBUF 2000  /* maximum size of buffered pseudo-keystrokes */
#define ARGVMAX 30  /* maximum number of files to open at startup */

WindowPtr ourWindow = 0;  /* needs to be a global so all funcs can get at it */




/* getenv is supposed to return the environmental variable defined as what[].
   since we're on a mac, we don't have environmental variables.  sigh.
   eventually I should put in a special case for $EXRC so that the proper
   initialization sequence could be grabbed from a resource.
*/
   
char *
getenv (const char *what) 
{
	return 0;
}

/* mch_write is the function that everybody in the universe who wants to draw
   a string is supposed to call.  we call our own drawtext, which in turn
   draws each character singly, but that's not important right now.
*/

void
mch_write(char_u *p, int len)
{
	drawtext (p, len);
}

/* here is the stuff for buffering what appear to be keystrokes but are really
   the mac interface making it look like stuff is being typed to vim.  typebuf
   holds the characters; buflen is how many of them there are still to be done.
   
   this stuff is called further down on, specifically in the part that is supposed
   to get keystrokes and which has been overloaded to handle the mac interface.
*/

static char typebuf[MAXTYPEBUF];
static int buflen = 0;

/* getfrombuf returns a character from the mac interface if there's one to be
   had, or zero if there is none.  yes, this means that we can't buffer nulls,
   but after all, this *is* a c program...
   
   as the character is returned, the other ones in the buffer are scooted
   forward and the number left is decremented.
*/

char_u 
getfrombuf() 
{
	int i;
	char_u rtn;
	
	if (buflen == 0) return 0;
	rtn = typebuf[0];

	for (i = 0; i < buflen; i++) {
		typebuf[i] = typebuf[i+1];
	}
	buflen--;
	return rtn;
}

/* addtobuf adds a single character to the typeahead buffer.  notice the
   skillful check of whether the buffer's already full before we add
   a character to it, and the not-so-skillful dumping of any characters
   over the limit into the bit bucket.
*/

void 
addtobuf (char_u what) 
{
	canbeep = 0;  /* don't beep in response to menu items */
	if (buflen < MAXTYPEBUF) {
		typebuf[buflen] = what;
		buflen++;
	}
}

/* addstrtobuf is the analogue of addtobuf, but for strings.  not surprisingly,
   it just calls addstrtobuf a whole bunch of times until it's done.
*/

void 
addstrtobuf (char_u* what) 
{
	while (*what) {
		addtobuf (*what);
		what++;
	}
}

/* mac filesystem support stuff.  the horror, the horror.
   either vim should be changed to know about macintosh file structure,
   or the mac should be changed to have a "normal" hierarchical directory
   structure like unix.  i fear neither would be much fun.
*/

/* getname gets the full pathname of a macintosh directory dirnum on
   volume vRef.  this is a ridiculous, awful way to do this, and ought
   to be done by sending the disk name, directory number, and filename
   to open() or fopen() rather than converting and converting back all
   the time.  but this would involve rewriting all of standard io,
   which i'm really not up to right now.
   
   vaguely based on an example in the comp.sys.mac.programmer faq,
   but changed around and probably made much less efficient in the process.
*/

/* THIS DOES NOT GET CALLED ANY MORE */

void 
getName (long dirnum, short vRef, Str255 where, long *parentdir) 
{
	CInfoPBRec paramBlock;
	Str255 tempstr;
	
	/* setting ioFDirIndex to -1 makes it search for the dirnum we passed
	   to it, instead of the (null) filename */
	   
	paramBlock.hFileInfo.ioCompletion = 0;
	paramBlock.hFileInfo.ioNamePtr = tempstr;
	paramBlock.hFileInfo.ioVRefNum = vRef;
	paramBlock.hFileInfo.ioFDirIndex = -1;
	paramBlock.dirInfo.ioDrDirID = dirnum;

	/* if this returns zero, we're looking at a directory that doesn't
	   exist.  just return null and get out of here, since there's
	   no sensible way to deal with this */
	   	
	if (PBGetCatInfoSync (&paramBlock) != 0) {
		where[0] = 0;
		return;
	}
	
	/* i'm not sure why i return parentdir in a variable, since the calling
	   function never uses it.  oh well, it saves a little stack space,
	   i guess... 
	   
	   anyway, if the parentdir was 1, we're at the root directory,
	   so cat the directotry name onto the return string and return.
	   
	   otherwise, we've still got a ways to go, so make a temporary copy
	   of the current directory name, call ourself recursively to handle
	   any lower directories, and *then* cat the directory name onto
	   the return string.  after enough times of this, we've either
	   run out of space in a Str255 or we've got a full path.
	   
	   if malloc fails, we don't save the string and just act like it
	   never existed.  if this happens, we've got worse problems to
	   deal with.
	   
	   since alloc() preserves at least 8k free, and tries to do 32k,
	   this shouldn't be too bad anyway.
	*/
	
	*parentdir = paramBlock.dirInfo.ioDrParID;
	
	p2cstr (tempstr);
	if (*parentdir == 1) {
		strcat ((char *)where, (char *)tempstr);
	} else {
		char *temp;
		
		if (temp = malloc (strlen ((char *)tempstr) + 5)) {
			strcpy (temp, (char *)tempstr);
			getName (*parentdir, vRef, where, parentdir);
			if (strlen ((char *)temp) + strlen ((char *)where) > 250) {
				*where = 0;
			} else {
				strcat ((char *)where, ":");
				strcat ((char *)where, temp);
			}
			free (temp);
		} 
	}
}

/* lastOpen is a Pascal string storing the last file opened, so that we can
   put something vaguely sensible in the Save As... dialog box.  Indeed,
   this will be utterly silly if other files have been opened since the
   ones we're trying to save, but it's probably better than nothing.
*/

/* actually this doesn't get used anymore, since we grab the file title
   out of the title bar.  oh well. */

static Str255 lastOpen = "\p";


/* pstrcpy is like strcpy but for pascal strings.  i'm surprised this
   isn't in a standard library.  maybe it is and i just don't know what
   it's called.  oh well.  anyway, we use this to return strings from 
   our fake StandardGetFile and StandardPutFile because it's stupid to
   convert to C strings and back again.
*/

void 
pstrcpy (Str255 dest, Str255 src) 
{
	int i;
	
	for (i = 0; i <= src[0]; i++) {
		dest[i] = src[i];
	}
}

/* this is a very silly hack.  blah.

   it seems that if you're running under system 7, the good old trick of
   using Standard File to get files that have been opened from the finder
   doesn't work anymore - *unless* you use SFPutFile and SFGetFile instead
   of the newfangled StandardGetFile and StandardPutFile.  Since I had already
   written code that used FSSpecs, these things call the old routines and
   then convert their return values into newstyle SFReplies.
   
   This should go away once I actually find out how to use AppleEvents
   so I can get these properly instead of the Finder having to patch
   traps all over the place.
*/

/* THIS DOES NOT GET CALLED ANYMORE since we're not sys6 compatible anyway
   and we can get files to open using appleevents.
*/

void 
ourSFPutFile (Str255 prompt, Str255 newname, StandardFileReply *reply) 
{
	Point ourPoint = {80, 40}; /* stupid to have to specify this... */
	SFReply theReply;
	Str255 ourString;
	
	SFPutFile (ourPoint, prompt, newname, 0, &theReply);
	reply->sfGood = theReply.good;
	pstrcpy (ourString, theReply.fName);
	FSMakeFSSpec (theReply.vRefNum, 0, ourString, &reply->sfFile);
}

/* likewise. */

void 
ourSFGetFile (void *proc, short numTypes, SFTypeList theTypes, StandardFileReply *reply) 
{
	Point ourPoint = {80, 40};
	SFReply theReply;
	Str255 ourString;
	
	SFGetFile (ourPoint, "\p", 0, numTypes, theTypes, 0, &theReply);
	reply->sfGood = theReply.good;
	pstrcpy (ourString, theReply.fName);
	FSMakeFSSpec (theReply.vRefNum, 0, ourString, &reply->sfFile);	
}

/* basename returns a pointer to the place within what[] where the last
   component of the path is.
*/

char *
basename (char *what) 
{
	char *here = what;

	while (strstr (here, ":")) here = strstr (here, ":") + 1;
	return here;
}

/* defaultname gives a filename to use as the default.  This it obtains from
   the title of the window, which should contain the name of the current
   vim subwindow.  used by getnewpath.
*/

StringPtr 
defaultname() 
{
	Str255 thename;
	static Str255 retval;
		
	if (ourWindow) {
		GetWTitle (ourWindow, thename);
		p2cstr (thename);
		if (strcmp ((char *) thename, "VIM - ") != 0) {
			strcpy ((char *) retval, basename ((char *) thename));
			c2pstr ((char *) retval);
		} else {
			strcpy ((char *) retval, "Untitled");
			c2pstr ((char *) retval);
		}
	} else retval[0] = 0;
	
	return (StringPtr) retval;
}

/* getnewpath is used to put up a standard file save dialog and return a C
   string that gives the path to it.  Not sure why I declared this as a Str255,
   but it's probably a reasonable thing to do because fopen's going to barf
   on longer paths anyway.
*/

void 
getnewpath (Str255 buf) 
{
	StandardFileReply reply;
	Str255 parent = "\p";
	long parentdir;
	
	/* put up the dialog.  if it's cancelled, just return a null string. */
	
	StandardPutFile ("\pSave as:", defaultname(), &reply);
	if (reply.sfGood == 0) {
		*buf = 0;
		return;
	}
	
	p2cstr (reply.sfFile.name);
	sprintf ((char *)buf, "%d:%ld:%s", reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name);
}

/* this does the same thing, but retrieving the name of an existing file. */

void 
getoldpath (Str255 buf) 
{
	StandardFileReply reply;
	SFTypeList ourtypes = {'TEXT', 0, 0, 0};  /* should we read other types? */
	Str255 parent = "\p";
	long parentdir;
	
	StandardGetFile (0, 1, ourtypes, &reply);
	if (reply.sfGood == 0) {
		*buf = 0;
		return;
	}
	
	p2cstr (reply.sfFile.name);
	sprintf ((char *)buf, "%d:%ld:%s", reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name);
	
	strcpy ((char *)lastOpen, (char *)reply.sfFile.name);
	c2pstr ((char *)lastOpen);
	return;
}

/* this is the appleevent file-opener routine, which gets called whenever we
   receive an open or print event.  if this is the first file, use :vi; otherwise
   use :new.  the menu-item open thing should probably call this too instead of
   doing it manually, but that's not a big deal.
*/

void
openFile (FSSpec what)
{
	Str255 buf;
	
	p2cstr (what.name);
	sprintf ((char *) buf, "%d:%ld:%s", what.vRefNum, what.parID, what.name);

	if (firstone) {
		addstrtobuf ((char_u *) "\033:vi ");
	} else {
		addstrtobuf ((char_u *) "\033:new ");
	}
	firstone = 0;
	
	addstrtobuf ((char_u *) buf);
	addstrtobuf ((char_u *) "\r");
}

/* here is our appleevent handler.  pretty, isn't it?  of course, remembering
   to declare AppleEvents as pointers (why can't Pascal take structs in
   function calls????) and the function as "pascal OSErr" took forever to
   figure out before it stopped crashing.  i guess you've always got to
   learn the hard way the first time...

   notice that although lots of things can generate errors, no one ever
   checks the return value from the appleevent dispatcher.  oh well.
*/

pascal OSErr
opendoc (AppleEvent *theEvent, AppleEvent *reply, long handlerRefCon)
{
	AEDescList docList;
	long itemsInList;
	long index;
	OSErr err;
	
	FSSpec myFSS;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	
	if (err = AEGetParamDesc (theEvent, keyDirectObject, typeAEList, &docList)) return err;
	if (err = AECountItems (&docList, &itemsInList)) return err;
	for (index = 1; index <= itemsInList; index++) {
		if (err = AEGetNthPtr (&docList, index, typeFSS, &keywd, &returnedType, (Ptr) &myFSS, sizeof (myFSS), &actualSize)) return err; 
		openFile (myFSS);
	}
	return 0;
}

/* do nothing when we open the app */

pascal OSErr
openapp (AppleEvent *theEvent, AppleEvent *reply, long handlerRefCon)
{
	return 0;
}

/* when we want to close the app, type a quit for all the windows. */

pascal OSErr
closeapp (AppleEvent *theEvent, AppleEvent *reply, long handlerRefCon)
{
	addstrtobuf ((char_u *) "\033:qall\r");
	return 0;
}

/* here is startup stuff.  for some reason, passing filenames to main()
   in a reasonable way doesn't work reasonably, so we have to do it
   with faked keystrokes.  sigh...
*/

/* some holders for our file variables.  note that newargc is one less than
   argc would be for the same number of files -- newargc is the *number of*
   files to be read.
*/

char *newargv[ARGVMAX];
int newargc;

/* what the heck is strdup doing here?  it's here because i needed it and
   it had to go somewhere.  i guess unix has spoiled me, having this sort
   of thing as a standard library.  anyway, all it does is return a pointer
   to memory which will then contain a *copy* of the C string, foo.  if
   malloc fails, we return 0 too.
*/

char *
strdup (char *foo) 
{
	char *new = malloc (strlen (foo) + 1);
	if (new) strcpy (new, foo);
	return new;
}

/* ah, here is the place where we get the files that are being passed to
   our mac app as if like command line arguments.  you'd think it'd be
   a simple thing, and indeed it would be if our app used real mac file
   system commands, but since we're using stdio we have to make everything
   into faked keystrokes.  we *still* aren't to GetChars yet... sigh.
   anyway, this chugs through each of the files and stuffs them in
   variables to be retrieved when we finally make it into the event
   loop, still a million miles from here...
   
   THIS GETS CALLED, BUT DOESN'T DO ANYTHING USEFUL under system 7.
   it's still here under a vague delusion that one day i might figure
   out what's breaking this under system 6 and get it working.
   
   it doesn't do any harm, anyway.
*/

void 
macmain() 
{
	short doWhat, fileCnt;
	
	CountAppFiles (&doWhat, &fileCnt);
	if (fileCnt == 0 || fileCnt > ARGVMAX) {
		newargc = -1;
		return;
	} else {
		int i;
		
		for (i = 1; i <= fileCnt; i++) {  /* this is Pascal, so 1-indexed */
			AppFile theFile;
			FSSpec FileRec;
			Str255 parent = "\p";   /* reinitialized each time through the loop */
			Str255 buf = "\p";
			long parentdir;
			char *here;

            /* get argv[i] and make a FSSpec from it. */
            
			GetAppFiles (i, &theFile);
			FSMakeFSSpec (theFile.vRefNum, 0, theFile.fName, &FileRec);

			/* once we've got the FSSpec, get its parent directory
			   and stuff the full path into a string.
			*/
			
			p2cstr (FileRec.name);
			sprintf ((char *)buf, "%d:%ld:%s", FileRec.vRefNum, FileRec.parID, FileRec.name);
	
	        /* since this is a local variable, make a copy of it and
	           stick it into argv[]. */
	           
			here = strdup ((char *) buf);
			newargv[i-1] = here;		

			ClrAppFiles (i);
		}
		
		newargc = fileCnt - 1;
		return;
	}
}

/* we're getting closer!  almost there!

   this is the mac routine to handle menu selections, and gets called
   by the event loop which is just below, disguised as a routine to
   read a character from the keyboard.
   
   everything that doMenu does (except for the About dialog) is accomplished
   by stuffing strings in a buffer, to be read later and executed as if
   they were genuine keystrokes.
   
   just about everything here is going to begin with a \033 (escape)
   so that it'll still work even if we were in the middle of inserting
   text or were sitting on the command line.
*/

void 
doMenu (long item) 
{
	int whichMenu = HiWord (item);
	int whichItem = LoWord (item);
	
	switch (whichMenu) {
		case 128: /* apple menu */
			switch (whichItem) {
				case 1:                   /* alain points out i should have
				                             #defined all these.  oh well. */
					Alert (128, 0);
				break;
				
				case 2: /* help */
					addstrtobuf ((char_u *)"\033:help\r");
				break;
				
				default:  /* it must be a desk accessory, so open it */
					{
						Str255 itemText;
						
						GetItem (GetMenu (128), whichItem, itemText);
						OpenDeskAcc (itemText);
					}
				break;
			}
		break;
		
		case 129: /* file menu */
			switch (whichItem) {
				case 1: /* new */
					addstrtobuf ((char_u *)"\033\027n");  /* escape control-W n */
				break;
				
				case 3: /* open */
					{
						Str255 pathname;
						
						getoldpath(pathname);
						if (*pathname) {
							addstrtobuf ((char_u *) "\033:new ");
							addstrtobuf ((char_u *) pathname);
							addstrtobuf ((char_u *) "\r"); 
						}
					}
				break;
				
				case 4: /* open same */
					{
						Str255 pathname;
						
						getoldpath (pathname);
						if (*pathname) {
							addstrtobuf ((char_u *) "\033:vi ");
							addstrtobuf ((char_u *) pathname);
							addstrtobuf ((char_u *) "\r");
						}
					}
				break;
				
				case 5: /* insert */
					{
						Str255 pathname;
						
						getoldpath (pathname);
						if (*pathname) {
							addstrtobuf ((char_u *) "\033:r ");
							addstrtobuf ((char_u *) pathname);
							addstrtobuf ((char_u *) "\r");
						}
					}
				break;
				
				case 7: /* close */
					addstrtobuf ((char_u *)"\033\:q\r");
				break;
				
				case 9: /* save */
					addstrtobuf ((char_u *)"\033:w\r");
				break;
				
				case 10: /* save as... */
					{
						Str255 pathname;
						
						getnewpath(pathname);
						if (*pathname) {
							addstrtobuf ((char_u *) "\033:w! ");
							addstrtobuf ((char_u *) pathname);
							addstrtobuf ((char_u *) "\r");
						}
					}
				break;
				
				case 12: /* page setup */
				break;
				
				case 13: /* print */
				break;
				
				case 15: /* quit */
					addstrtobuf ((char_u *)"\033:qall\r");
				break;
			}
		break;
		
		case 130: /* edit menu */
			switch (whichItem) {
				case 1: /* undo */
					addstrtobuf ((char_u *)"\033u");
				break;

				case 2: /* redo */
					addstrtobuf ((char_u *)"\033\022");  /* control-R */
				break;
				
				case 4: /* cut */
				break;
				
				case 5: /* copy */
				break;
				
				case 6: /* paste */
				break;
				
				case 8: /* prev */
					addstrtobuf ((char_u *)"\033\027k");
				break;
				
				case 9: /* next */
					addstrtobuf ((char_u *)"\033\027j");
				break;
			}
		break;
		
		case 131: /* font menu */
			{
				Str255 theName;
				short theNum;
				
				GetItem (GetMenu (131), whichItem, theName);
				GetFNum (theName, &theNum);
				
				SetPort (ourWindow);
				TextFont (theNum);
			}
		break;
		
		case 132: /* size menu */
			{
				Str255 theSize;
				long numSize;
				
				GetItem (GetMenu (132), whichItem, theSize);
				StringToNum (theSize, &numSize);
				
				SetPort (ourWindow);
				TextSize (numSize);
			}
		break;
	}

	switch (whichMenu) {
		case 131:
		case 132:  /* fix the font stuff... */
			{
				FMInput getFont;
				FMOutPtr outFont;
				Point myPoint = {1, 1};
				
				getFont.family = qd.thePort->txFont;
				getFont.size = qd.thePort->txSize;
				getFont.face = 0;
				getFont.needBits = 0;
				getFont.device = 0;
				getFont.numer = myPoint;
				getFont.denom = myPoint;
				
				outFont = FMSwapFont (&getFont);
				
				drawCursor();  /* erase it before we change the window */
				
				charhigh = outFont->ascent + outFont->descent + outFont->leading;
				
				/* why do we use the width of 'e' as our cell width?
				   well... um.... it's a pretty common character....
				   
				   actually, it's because if we used a character as wide as 'm'
				   there'd be big gaps between most of the characters, and if
				   we used one as narrow as 'i' everything would be all squished
				   together.  e is kind of middleish in its size, so it seemed
				   like a good one to use.
				*/
				
				charwid = CharWidth ('e');
				
				/* why don't we just check the monospacedness field in the
				   font info?  because there are some fonts that aren't marked
				   monospaced but really are, and if we can draw them quickly
				   we might as well.
				   
				   any font that isn't monospaced and has 'M' and 'l' the same
				   width is a pretty demented one.
				*/
				
				aremonospaced = (CharWidth ('M') == CharWidth ('l'));
				charascent = outFont->ascent;
				
				set_winsize ((ourWindow->portRect.right - 4) / charwid, ourWindow->portRect.bottom / charhigh, FALSE);
				scroll_region_reset();
									
				drawCursor();  /* put the cursor back */
			}
	}
	HiliteMenu (0);
}

/* this sets our current font to font[].  probably the menu item should call
   this instead of doing it itself, but instead, this just gets called when
   someone :sets term.  we check if the font is a number and if so, treat it
   as a size instead.
*/

void 
setfont (char *font)
{			
	FMInput getFont;
	FMOutPtr outFont;
	Point myPoint = {1, 1};
	short result;
	
	if (ourWindow == 0) return;
	if (font == 0) return;
	if (strlen (font) == 0) return;
	
	if (result = atoi (font)) {
		SetPort (ourWindow);
		TextSize (result);
	} else {
		Str255 foo;
		
		strcpy ((char *) foo, font);
		c2pstr ((char *) foo);
		GetFNum (foo, &result);
				
		SetPort (ourWindow);
		if (result) TextFont (result);
		else {
			char foo[255];
			
			sprintf (foo, "Can't find a font called %s.", font);
			emsg (foo);
			return;
		}
	}
	
	getFont.family = qd.thePort->txFont;
	getFont.size = qd.thePort->txSize;
	getFont.face = 0;
	getFont.needBits = 0;
	getFont.device = 0;
	getFont.numer = myPoint;
	getFont.denom = myPoint;
	
	outFont = FMSwapFont (&getFont);
	
	charhigh = outFont->ascent + outFont->descent + outFont->leading;
	charwid = CharWidth ('e');
	aremonospaced = (CharWidth ('M') == CharWidth ('l'));
	charascent = outFont->ascent;
	
	set_winsize ((ourWindow->portRect.right - 4) / charwid, ourWindow->portRect.bottom / charhigh, FALSE);
	scroll_region_reset();
}

/* we're almost there!  we're almost there!  all we've got to do is draw the
   cursor, and... */
   
/* drawCursor() draws the cursor on the screen.  if you like underlines,
   blinking blocks, whatever, this is what to change to make it do that.
   i like a solid block so that's what this does.
   
   notice that the same routine is called to *draw* and to *erase* the cursor,
   so either you have to be symmetric in your graphics operations or keep
   track of whether you drew or erased the cursor last time so you can do
   the right thing.
*/

void 
drawCursor() 
{
	Rect thisrect;
	
	/*if (cursorvis) {*/
		thisrect.top = cursorline * charhigh - 1;
		thisrect.left = cursorcol * charwid + 2;
		thisrect.right = thisrect.left + charwid + 1;
		thisrect.bottom = thisrect.top + charhigh + 2;
		
		SetPort (ourWindow);
		InvertRect (&thisrect);
	/*}*/
}

/* look!  it's GetChars()!!!!!!

   the official specs are as follows:
 * GetChars(): low level input funcion.
 * Get a characters from the keyboard.
 * If time == 0 do not wait for characters.
 * If time == n wait a short time for characters.
 * If time == -1 wait forever for characters.
 *
 * Return number of characters read.
 
   but in practice, to keep the cursor from blinking insanely and being
   really annoying and flickery, we only return a few null keystrokes
   after each real one (to make sure that text being inserted gets
   drawn!  if we don't return a null, it doesn't happen until a backspace
   or return) and then just sit WaitNextEventing until something better
   comes along.
*/

int
GetChars(buf, maxlen, time)
	char_u	*buf;
	int		maxlen;
	int		time;				/* milli seconds */
{
	EventRecord theEvent;
	WindowPtr whichWindow;
	
	int ticks = time * 60 / 1000;  /* we need mac ticks, not milliseconds */

	static long now = 0;   /* when we got our last key */
	static int idles = 0;   /* how many null events we've sent since the key */
	static long lastkey = 0;  /* last real *or* virtual keystroke time */
	static int justwrote = 0;  /* did we already write the script file?  */
	static int areselecting = 0; /* are we mouse-selecting text? */
	static int aredragging = 0; /* and are we moving the cursor with the mouse? */
	int i;
	
	if (ticks < 0) ticks = 60;  /* forever is one second.  dunno why. */
	
	/* Ugly Hack Number One.  if we have command line arguments that still
	   haven't been processed, spew them into the keystroke buffer so we'll
	   get them the next time we go looking for a key.
	*/
	
	if (newargc >= 0) {
		static int firstone = 1;
		
		if (firstone) {
			sprintf ((char *) buf, ":vi %s\r", newargv[newargc]);
		} else {
			sprintf ((char *) buf, ":new %s\r", newargv[newargc]);
		}
		firstone = 0;
		
		newargc--;
		return strlen ((char *) buf);
	}

	/* draw the cursor.  this happens *only* when we're delaying or waiting
	   for a key.  make sure to erase the cursor at any abnormal function
	   exits, or you'll be left with bunches of extra cursors lying around.
	*/
	
	fixpending();
	drawCursor();	
	
	*buf = 0;
	
	/* now we're going to do what's basically a standard macintosh
	   event loop, but stuck in the middle of a get keystroke routine.
	   keep waitnexteventing until something happens that's worthy
	   of a keystroke return to vim.
	*/
	
	if (Rows != shouldrows || Columns != shouldcols) {
		drawCursor();
		
		mch_set_winsize();
		scroll_region_reset();
		
		drawCursor();
	}
	
	while (*buf == 0) {		
		SetCursor (&arrow);

		/* next item of business: see if we've got any buffered characters
		   that we need to act like we're typing.  perhaps this should override
		   the keys, but it works well enough this way, and has the bonus of
		   giving visual feedback of the faked keystrokes as they happen.
		*/
		
		if (buflen) {
			*buf = getfrombuf();  /* getfrombuf() gets from our private buffer,
			                         not the buffer of keys to send to vim */
			buf[1] = 0;
			drawCursor();
			idles = 0;
			lastkey = TickCount();
			justwrote = 0;
			return 1;
		}

		if (aredragging) {
			if (StillDown()) { 
				Point ourPoint;
				GetMouse (&ourPoint);
				
				if ((cursorline != Rows) && aredragging) {
					int wantx, wanty;
					
					GlobalToLocal (&theEvent.where);
					
					wantx = (ourPoint.h - 2) / charwid;
					wanty = (ourPoint.v) / charhigh;
					
					if (wantx < 0) wantx = 0;
					
					if (wanty != cursorline) {
						if (wanty < cursorline) {
							addtobuf ((char_u) '\036'); /* up */
							wanty++;
						} else if (wanty > cursorline) {
							addtobuf ((char_u) '\037');  /* down */
							wanty--;
						}
					} else {	
						if (wantx < cursorcol) {
							addtobuf ((char_u) '\034');  /* left */
							wantx++;
						} else if (wantx > cursorcol) {
							addtobuf ((char_u) '\035');  /* right */
							wantx--;
						}
					}
				}
			} else { /* not stilldown */
				aredragging = 0;
			}
		} else { /* not aredragging */
			aredragging = 0;
			
			if (WaitNextEvent (everyEvent, &theEvent, ticks, 0)) {
				switch (theEvent.what) {
					case kHighLevelEvent:
						AEProcessAppleEvent (&theEvent);
					break;
					
					case mouseDown:
						switch (FindWindow (theEvent.where, &whichWindow)) {
							case inSysWindow:
								SystemClick (&theEvent, whichWindow);
							break;
							
							case inMenuBar:
								doMenu (MenuSelect (theEvent.where));
							break;
							
							case inDrag:
								DragWindow (whichWindow, theEvent.where, &qd.screenBits.bounds);
							break;
							
							case inZoomIn:
								ZoomWindow (whichWindow, inZoomIn, TRUE);
								set_winsize ((whichWindow->portRect.right - 4) / charwid,
								             whichWindow->portRect.bottom / charhigh,
								             FALSE);
								scroll_region_reset();
							break;
							
							case inZoomOut:
								ZoomWindow (whichWindow, inZoomOut, TRUE);
								set_winsize ((whichWindow->portRect.right - 4) / charwid,
								             whichWindow->portRect.bottom / charhigh,
								             FALSE);
								scroll_region_reset();
							break;
							
							case inGrow:
								{
									long eep;
									Rect ourRect;
									
									ourRect.top = 10 * charwid + 4;
									ourRect.left = 5 * charhigh;
									ourRect.bottom = qd.screenBits.bounds.bottom;
									ourRect.right = qd.screenBits.bounds.right;
									
									drawCursor();
									
									eep = GrowWindow (whichWindow, theEvent.where, &ourRect);
									if (eep) {
										SizeWindow (whichWindow, LoWord (eep), HiWord (eep), TRUE);
										set_winsize ((LoWord (eep) - 4) / charwid, HiWord (eep) / charhigh, FALSE);
										scroll_region_reset();
									}
									
									drawCursor();								
								}
							break;
							
							case inGoAway:
								if (TrackGoAway (whichWindow, theEvent.where)) addstrtobuf ((char_u *) ":q\r");
							break;
	
							case inContent:
								if (whichWindow != FrontWindow()) {
									SelectWindow (whichWindow);
								} else {
									/* don't fake keys if we're at the bottom line,
									   because that means we're editing a : string or
									   in an error message or something.
									   
									   otherwise, try to move the cursor to where
									   the user just clicked so that they can have
									   some vague semblance of a graphics-based
									   interface.  this will break if there are
									   lines that are broken between screen lines, but
									   alas, such is life.  sigh.
									   
									   i like editors that go by screen line instead
									   of file lines, but apparently this just
									   doesn't happen in the unix world...
									*/
									
									if (cursorline != Rows) {
										int wantx, wanty;
										
										GlobalToLocal (&theEvent.where);
										
										wantx = (theEvent.where.h - 2) / charwid;
										wanty = (theEvent.where.v) / charhigh;
										
										if (wantx < 0) wantx = 0;
										aredragging = 1;
										
										if (wantx == cursorcol && wanty == cursorline) {
											if (!areselecting) {
												areselecting = 1;
												addstrtobuf ((char_u *) "\033v"); /* begin selection */
											} else {
												areselecting = 0;
												addtobuf ((char_u) '\033'); /* end selection */
											}
										} else {
											if (areselecting) {
												areselecting = 0;
												addtobuf ((char_u) '\033');	
											}	
											if (wanty != cursorline) {
												if (wanty < cursorline) {
													addtobuf ((char_u) '\036'); /* up */
													wanty++;
												} else if (wanty > cursorline) {
													addtobuf ((char_u) '\037');  /* down */
													wanty--;
												}
											} else {	
												if (wantx < cursorcol) {
													addtobuf ((char_u) '\034');  /* left */
													wantx++;
												} else if (wantx > cursorcol) {
													addtobuf ((char_u) '\035');  /* right */
													wantx--;
												}
											}
										}
									}
								}
							break;
						}
					break;
					
					case keyDown:
					case autoKey:
							{
								if (theEvent.modifiers & cmdKey) {
									doMenu (MenuKey (theEvent.message & charCodeMask));
									break;  /* case */
								}
							}
							canbeep = 1;
							firstone = 0;
							*buf = theEvent.message & charCodeMask;
							idles = 0;
							lastkey = TickCount();
							justwrote = 0;
							areselecting = 0;
							now = TickCount();
							{
								Point myPoint = {0,0};
							}
					break;
					
					case updateEvt:
						BeginUpdate ((WindowPtr) theEvent.message);
							/* screen update should be in between here, but we're
							   going to redraw the whole screen, not just the piece
							   that got overwritten.
							*/
						EndUpdate ((WindowPtr) theEvent.message);
						
						/* repaint the whole screen, because if we set the clip
						   region with BeginUpdate and EndUpdate and end up scrolling
						   while only part of the window can be drawn on, things get
						   massively ugly.  the refresh is now fast enough we can
						   stand to redo the whole thing. */
	
						screenclear();
						check_winsize();		/* always check, to get p_scroll right */
						if (State == HELP)
							(void)redrawhelp();
						else if (!starting)
						{
							int tmp = RedrawingDisabled;
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
						drawCursor();
					break;
				}
			} else {
				/* we get here if there isn't a single event waiting in the macintosh
				   event queue.  what a bizarre happening, huh? */
				   
				/* if we've passed the amount of time we're supposed to
				   wait for a key, then we've officially idled once.
				   increment idles and reset the time to celebrate.
				*/
				
				if (TickCount() > now + ticks) {
					now = TickCount();
					idles++;
				}
				
				/* since we're officially idling, we should tell vim about it
				   so that we'll get screen redraws as we type.  this was really
				   annoying to find, so i went several days typing blind until
				   i found out i should be sending idles.  oh well.
				*/
				
				if (idles < 2) {  /* send a few idles after each keystroke */
					drawCursor();
					return 0;
				}
				
				/* spew some stuff out to disk to get it out of memory (we hope).
				   if the memory is getting low, change the window title to
				   reflect this and warn the user, though it's probably too
				   late if this is already happening.
				*/
				
				if (lastkey && (justwrote == 0)) {
					if (TickCount() > lastkey + 4 * 60) {
						justwrote = 1;
						updatescript(0);
						if (mch_avail_mem (TRUE) < 10000) {
							mch_settitle ((char_u *)"Warning -- memory is very low!", 0);
						}
					}
				}
				
				/* keep the int from rolling over */
				
				if (idles > 20000) idles = 20000;
			}
		}
	}
 	buf[1] = 0;
	drawCursor();
 	return 1;
}

/* i don't think this ever gets called. actually, i take that back, because
   there's a link error if it's not there.  of course, this could be
   just because of a prototype that never really gets used.
*/

/*
 * return non-zero if a character is available
 */
int
mch_char_avail()
{
	return (WaitForChar(100L) != 0);
}


/* return the amount of memory left to be had in our own little universe.

   we need to call MaxApplZone() or else be left with a pathetically small
   result that will set the alarm bells running.  i'm not sure whether it's
   bad to call it more than once, but it's a waste of function calls if
   nothing else, so we only do it once.
*/

long
mch_avail_mem(special)
	int		special;
{
	long foo;
	static int alreadyMaxed = 0;

	if (alreadyMaxed == 0) {
		MaxApplZone();
		alreadyMaxed = 1;
	}
	
	foo = MaxBlock();
	return foo;
}

/* vim_delay is so we can see the cursor flash when we type a right paren.
   we intentionally draw the cursor before waiting and erase it afterwards,
   because since we're not waiting for a character there isn't otherwise
   one on the screen.
*/

void
vim_delay()
{
	long m;
	
	fixpending();
	drawCursor();	
	Delay (20, &m);
	drawCursor();
}

/* this would suspend us if we could suspend, but we can't.  maybe this should
   bring the finder to the front instead, but that's easy enough to do with
   a click...
*/

void
mch_suspend()
{
	; /* suspend us */
}

/* initialize the window system.

   actually, since this is our first chance, initialize all the other managers
   that good little mac programs are supposed to start up too.
   
   and set a few random variables that needed to get initialized somewhere.
*/

void 
mch_windinit()
{
	Handle menuBar;
	long vers;
	
	InitGraf (&qd.thePort);
	InitFonts();
	FlushEvents (everyEvent, 0);
	InitWindows();
	InitMenus();
	InitDialogs (0);
	InitCursor();
	
	ourWindow = GetNewWindow (128, 0, (GrafPtr) -1);
	Rows = ourWindow->portRect.bottom / charhigh;
	Columns = (ourWindow->portRect.right - 4) / charwid;
	
	menuBar = GetNewMBar (128);
	SetMenuBar (menuBar);
	DisposHandle (menuBar);
	AddResMenu (GetMHandle (128), 'DRVR');
	DrawMenuBar();
	AddResMenu (GetMHandle (131), 'FONT');
	
	SetPort (ourWindow);
	TextFont (4);
	TextSize (9);
	
	_fcreator = 'VIM!';  /* force output to VIM!/TEXT type */

	aRegion = NewRgn();
	
	if (!Gestalt (gestaltSystemVersion, &vers)) {
		if (vers >= 0x0700) {		/* will crash on sys6 if we don't check */
			AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments, opendoc, 0, FALSE);
			AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments, opendoc, 0, FALSE);
			
			AEInstallEventHandler (kCoreEventClass, kAEOpenApplication, openapp, 0, FALSE);
			AEInstallEventHandler (kCoreEventClass, kAEQuitApplication, closeapp, 0, FALSE);
		}
	}
}


/* this is supposed to exit if we don't have control over our terminal.  since we're
   on the console, it seems pretty guaranteed....
*/

void
check_win(argc, argv)
	int argc;
	char **argv;
{
	/* ok, we'd better be in a window of our own... */
}

/*
 * fname_case(): Set the case of the filename, if it already exists.
 *				 This will cause the filename to remain exactly the same.
 */

void
fname_case(name)
	char_u	*name;
{
	return;
}

/*
 * set the title of our window
 * icon name is ignored
 */
 
/* this does *not* set the title to exactly what the calling function wants,
   because it's really ugly if we do.  follow the mac standard of showing
   just the filename in the title, not "VIM - " and the full path.
*/
 
void
mch_settitle(title, icon)
	char_u	*title;
	char_u	*icon;
{
	if (ourWindow) {
		Str255 newTitle;
		
		strcpy ((char *)newTitle, basename ((char *) title));
		c2pstr ((char *)newTitle);
		SetWTitle (ourWindow, newTitle);
	}
}

/* this should restore the window title when we quit, but since it's
   our window it's kinda silly anyway.  oh well.
   This probably never gets called anyway.
*/

void
mch_restore_title(which)
	int which;
{
	mch_settitle((char_u *)"The old title", 0);
}

/*
 * Get name of current directory into buffer 'buf' of length 'len' bytes.
 * Return OK for success, FAIL for failure.
 */
	int
vim_dirname(buf, len)
	char_u		*buf;
	int			len;
{
	strcpy ((char *)buf, ":");
	return OK;
}

/*
 * get absolute filename into buffer 'buf' of length 'len' bytes
 *
 * return FAIL for failure, OK otherwise
 */
	int
FullName(fname, buf, len)
	char_u		*fname, *buf;
	int			len;
{
	strcpy ((char *)buf, (char *)fname);
	return OK;
}

/*
 * return TRUE is fname is an absolute path name
 */
	int
isFullName(fname)
	char_u		*fname;
{
	return (STRCHR (fname, ':') != NULL);
}


/*
 * get file permissions for 'name'
 */

/* if the file is readable, we return 666, otherwise an error.
   we should probably also check for writeability, but it seems
   bad to open the file for writing just for that.
   
   probably should stat (or its equivalent) instead, but this works.
*/

long
getperm(name)
	char_u		*name;
{
	FILE *foo = fopen ((char *)name, "r");

	if (foo) {
		fclose (foo);
		return 0666;
	}
	return -1;
}

/*
 * set file permission for 'name' to 'perm'
 *
 * return FAIL for failure, OK otherwise
 */
 
/* we're blissfully ignorant of mac file system permissions.  just act like
   we were successful and continue merrily onward.
*/

	int
setperm(name, perm)
	char_u		*name;
	long		perm;
{
	return OK;
}

/*
 * return FALSE if "name" is not a directory
 * return TRUE if "name" is a directory.
 * return -1 for error.
 */

/* mac assumption: if we can read it, it's a file.  otherwise, it's
   a directory. */
   
int
isdir(name)
	char_u		*name;
{
	FILE *test = fopen ((char *)name, "r");
	if (test) {
		fclose (test);
		return FALSE;
	}
	return -1;
}

/*
 * Careful: mch_windexit() may be called before mch_windinit()!
 */
	void
mch_windexit(r)
	int 			r;
{
	
/*	if (ourWindow) {
		SetPort (ourWindow);
		MoveTo (20, 20);
		DrawString ("\pShutting down!");
	}
*/	
	ml_close_all();
	exit (1);
}


/*
 * Function mch_settmode() - Convert the specified file pointer to 'raw' or 'cooked'
 * mode. This only works on TTY's.
 *
 * Raw: keeps DOS from translating keys for you, also (BIG WIN) it means
 *		getch() will return immediately rather than wait for a return. You
 *		lose editing features though.
 *
 * Cooked: This function returns the designate file pointer to it's normal,
 *		wait for a <CR> mode. This is exactly like raw() except that
 *		it sends a 0 to the console to make it back into a CON: from a RAW:
 */
	void
mch_settmode(raw)
	int			raw;
{
	return;
}

/*
 * set screen mode, always fails.
 */
	int
mch_screenmode(arg)
	char_u	 *arg;
{
	EMSG("Screen mode setting not supported");
	return FAIL;
}

/*
 * try to get the real window size
 * return FAIL for failure, OK otherwise
 */
	int
mch_get_winsize()
{
	return FAIL;
}

/*
 * try to set the real window size
 */
	void
mch_set_winsize()
{
	SizeWindow (ourWindow, Columns * charwid + 4, Rows * charhigh, TRUE);
	shouldrows = Rows;
	shouldcols = Columns;
}


/*
 * call shell, return FAIL for failure, OK otherwise
 */
	int
call_shell(cmd, filter, cooked)
	char_u	*cmd;
	int		filter;		/* if != 0: called by dofilter() */
	int		cooked;
{
	; /* suspend us ... */
}

/* dunno what this does.  copied from amiga.c */

	void
FreeWild(num, file)
	int		num;
	char_u	**file;
{
	if (file == NULL || num == 0)
		return;
	while (num--)
		free(file[num]);
	free(file);
}

/* again, don't really know what exactly this is supposed to do, so just act
   like it didn't work and look the other way.  sigh.
*/

/*
 * ExpandWildCard() - this code does wild-card pattern matching using the arp
 *					  routines. This is based on WildDemo2.c (found in arp1.1
 *					  distribution). That code's copyright follows :
 */


	int
ExpandWildCards(num_pat, pat, num_file, file, files_only, list_notfound)
	int 			num_pat;
	char_u		  **pat;
	int 		   *num_file;
	char_u		 ***file;
	int			files_only;
	int			list_notfound;
{
	return FAIL;
}

/* copied from... where? termlib.c, maybe?  probably. */

/*
 * Check if buf[] begins with a terminal key code.
 * Return 0 for no match, -1 for partial match, > 0 for full match.
 * With a match the replacement code is put in buf[0], the match is
 * removed and the number characters in buf is returned.
 *
 * Note: should always be called with buf == typestr!
 */
	int
check_termcode(buf)
	char_u	*buf;
{
	char_u 	**p;
	int		slen;
	int		len;

	len = STRLEN(buf);
	for (p = (char_u **)&term_strings.t_ku; p != (char_u **)&term_strings.t_undo + 1; ++p)
	{
		if (*p == NULL || (slen = STRLEN(*p)) == 0)		/* empty entry */
			continue;
		if (STRNCMP(*p, buf, (size_t)(slen > len ? len : slen)) == 0)
		{
			if (len >= slen)		/* got the complete sequence */
			{
				len -= slen;
					/* remove matched chars, taking care of noremap */
				del_typestr(slen - 1);
					/* this relies on the Key numbers to be consecutive! */
				buf[0] = K_UARROW + (p - (char_u **)&term_strings.t_ku);
				return (len + 1);
			}
			return -1;				/* got a partial sequence */
		}
	}
	return 0;						/* no match found */
}

