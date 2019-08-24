/* liar.c

   by eric
   19 apr

   i really should be doing real work...

*/

#include <stdio.h>
#include <stdlib.h>

main (int argc, char *argv[]) {
  char *file;
  char supposed[400];

  if (argc < 3) {
    fprintf (stderr, "Usage: %s supposed-name real-name [args]\n", argv[0]);
    exit (1);
  }
  file = argv[2];
  strcpy (supposed, argv[1]);
  while (strlen (supposed) < 300) strcat (supposed, "    ");

  argv[2] = supposed;

  execvp (file, argv+2);
  fprintf (stderr, "%s: Unable to load %s\n", file);
}
