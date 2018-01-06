#include <stdio.h>
#include <stdlib.h>

#define YEARS 3

enum compound_style { daily, monthly, quarterly };

struct bank_rate {
	char *bankname;
	double rate;
	compound_style how;
};

struct bank_rate banks[] = {
	"Bank One",		7.85,	daily,
	"Merchant's",		8.10,	monthly,
	"Union Federal",	8.24,	quarterly,
	"Railroadmen's",	8.20,	quarterly,
};
size_t nbank = sizeof (banks) / sizeof (banks[0]);

char *
compound_name (compound_style cs)
{
	switch (cs) {
		case daily:
			return "Daily";
		case monthly:
			return "Monthly";
		case quarterly:
			return "Quarterly";
		default:
			return "*** error ***";
	}
}
