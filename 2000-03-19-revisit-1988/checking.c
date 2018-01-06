#include <stdio.h>
#include <stdlib.h>

#define LINE_MAX 1000

int
main()
{
	char s[LINE_MAX];
	double balance;
	int i;

	printf ("---------------------------------------------------------\n");
	printf ("\n");
	printf ("Bank Statement for October, 1988\n");
	printf ("\n");

	printf ("Starting balance:            $ ");
	fgets (s, LINE_MAX, stdin);
	balance = atof (s);
	printf ("%10.2f\n", balance);
	printf ("\n");

	for (i = 1; fgets (s, LINE_MAX, stdin); i++) {
		double transaction = atof (s);

		printf ("Transaction %3d - ", i);
		if (transaction >= 0)
			printf ("Check..... $ %10.2f\n",  transaction);
		else
			printf ("Deposit... $ %10.2f\n", -transaction);

		balance -= transaction;
	}

	printf ("\n");
	printf ("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	printf ("\n");
	printf ("Your ending balance is       $ %10.2f\n", balance);
}
