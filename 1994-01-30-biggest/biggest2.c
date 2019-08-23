/* biggest.c

   eric fischer
   enf1

   slightly after midnight
   31 jan 1994
*/

#include <stdio.h>
#define TRUE 1
#define FALSE 0

int max (int first, int second) {
  if (first > second) return first;
  return second;
}

main() {
  int biggest = 0;
  char theString[256];
  int nextInt;
  char firstTime = TRUE;

  while (scanf ("%s", &theString) != EOF) {
    if (sscanf (theString, "%d", &nextInt)) {
      if (firstTime) {
        biggest = nextInt;
        firstTime = FALSE;
      } else {
        biggest = max (nextInt, biggest);
      }
    } else {
      printf ("You've got to give me a number!\n");
    }
  }
  printf ("The biggest was %d\n", biggest);
}
