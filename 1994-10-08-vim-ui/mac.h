/* mac.h

   set up stuff to find the right things on the mac
*/

/*
 * Names for the EXRC, HELP and temporary files.
 * Some of these may have been defined in the makefile.
 */

#ifndef SYSVIMRC_FILE
# define SYSVIMRC_FILE	"s:_vimrc"
#endif

#ifndef SYSEXRC_FILE
# define SYSEXRC_FILE	"s:_exrc"
#endif

#ifndef VIMRC_FILE
# define VIMRC_FILE		"_vimrc"
#endif

#ifndef EXRC_FILE
# define EXRC_FILE		"_exrc"
#endif

#ifndef VIM_HLP
# define VIM_HLP		"vim.hlp"
#endif

#ifndef DEF_DIR
# define DEF_DIR		">t:"
#endif

#define TMPNAME1		"viXXXXXX"
#define TMPNAME2		"voXXXXXX"
#define TMPNAMELEN		8

#ifndef MAXMEM
# define MAXMEM			0x7fffffff		/* will be reduced to maxmemtot at runtime */
#endif
#ifndef MAXMEMTOT
# define MAXMEMTOT		0		/* decide in set_init */
#endif

#define BASENAMELEN		32		/* MacOS */

void outchar (char_u);
long mch_avail_mem();

#include <unix.h>


#define fopen myfopen
#define rename myrename
#define remove myremove
#define open myopen
#define stat mystat

/* we need to restate struct stat because we just #defined stat to be mystat,
   hence the declaration in stat.h won't be noticed...
*/

struct stat {				/* "inode" information returned by stat/fstat */
	dev_t		st_dev;							// device of the inode
	ino_t		st_ito;							// inode number
	short		st_mode;						// mode bits
	short		st_nlink;						// number of links to file
	int			st_uid;							// owner's user id
	int			st_gid;							// owner's group id
	dev_t		st_rdev;						// for special files [ignored]
	off_t		st_size;						// file size in characters
	time_t	st_atime;						// time last accessed
	time_t	st_mtime;						// time last modified
	time_t	st_ctime;						// time originally created
};

int myrename (char *old, char *new);
int myremove(const char *s);
FILE *myfopen(const char *filename, const char *mode);
int mystat(char *filename, struct stat *sr);
int myopen(char *filename, int mode);

/* normal and invert are already taken by some mac library, it seems.... */

#define normal ___normal
#define invert ___invert

