/* format.c
   eric fischer

   to nicely format the output of rusers
   usage: rusers -l whatever | expand | format folks
*/

#include <stdio.h>


myputchar (theChar)

  char theChar;

{
  if (theChar) putchar(theChar);
  else         putchar(' ');
}

int readline (theFile, theString)

  FILE *theFile;
  char *theString;

{
  int count;
  char theChar;

  count = 0;
  while ((theChar = getc (theFile)) != '\n') {
    if (theChar == EOF) return 0;
    theString[count++] = theChar;    
  }
  theString[count++] = 0;
  return count;
}

substring (string, start, end)

  char *string;
  int start, end;

{
  while (start <= end) {
    myputchar (string[start]);
    start++;
  }
}

dotstring (string, start, end)

  char *string;
  int start, end;

{
  char done;
  done = 0;
  while (start <= end) {
    if (string[start] == '.') done=1;
    if (done) myputchar (' ');
    else      myputchar (string[start]);
    start++;
  }
}

fifteenPastSpace (string)

  char *string;

{
  int counter = 0;
  int next;
  char done = 0;

  while (string[counter++] != ' ') ;
  for (next = 0; next < 15; next++) {
    if (string[counter+next] == 0) done = 1;
    if (done == 0) myputchar (string[counter+next]);
    else           myputchar (' ');
  }

}

int compare (string1, string2)

  char* string1;
  char* string2;

{
  int counter;

  counter = 0;
  while (string1[counter] && string2[counter]) {
    if (string1[counter] != string2[counter]) return 0;
    if ((string2[counter] == ' ') && 
        (string1[counter] == ' ')) return 1;
    counter ++;
  }
  return 1;
}

main(argc, argv) 

  int argc;
  char *argv[];

{
  char *theString;
  char *otherString;
  FILE *theFile;

  theString = malloc (256);
  otherString = malloc (256);

  if (argc != 2) {
    printf ("Format must receive exactly one argument.\n");
    exit (1);
  }

  while (readline (stdin, theString) != 0) {

    theFile = fopen (argv[1], "r");
    while (readline (theFile, otherString)) {
      if (compare (theString, otherString)) {
        fifteenPastSpace (otherString);
        printf (" ");
        substring (theString, 0,7);
        printf ("  ");
        dotstring (theString, 9,17);
        printf ("  ");
        substring (theString, 31,42);
        printf ("  ");
        substring (theString, 52,69);
        printf ("  ");
        substring (theString, 46,51);
        printf ("\n");
      }
    }
    fclose (theFile);

  }

}
