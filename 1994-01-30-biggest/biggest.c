/* biggest.c

   eric fischer
   enf1

   just shortly before midnight
   30 jan 1994
*/

#include <stdio.h>

int max (int first, int second) {
  if (first > second) return first;
  return second;
}

main() {
  int biggest = 0;
  char theString[256];
  int nextInt;

  while (scanf ("%s", &theString) != EOF) {
    if (sscanf (theString, "%d", &nextInt)) {
      biggest = max (nextInt, biggest);
    } else {
      printf ("You've got to give me a number!\n");
    }
  }
  printf ("The biggest was %d\n", biggest);
}
