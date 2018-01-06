#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

double
fahr2celc (double f)
{
	return (f - 32.0) * 5.0 / 9.0;
}

#define LINE_MAX 1000

int
main()
{
	char s[LINE_MAX];

	if (isatty (0)) {  /* don't be so chatty if noninteractive */
		printf ("Please type a temperature in Fahrenheit (or type\n");
		printf ("an end-of-file to end) and it will be converted to\n");
		printf ("Celcius.\n\n");
	}

	while (1) {
		if (isatty (0)) {
			printf ("Fahrenheit degrees: ");
			fflush (stdout);
		}

		if (!fgets (s, LINE_MAX, stdin))
			break;

		printf ("Celcius degrees:    %.2f\n", fahr2celc (atof (s)));
	}
}
