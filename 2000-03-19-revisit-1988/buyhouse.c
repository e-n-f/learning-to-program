#include <stdio.h>
#include <stdlib.h>

struct houseinfo {
	double price;
	double heat_per_year;
	double tax_rate;
};

struct houseinfo houses[] = {
	67000.00,	2300,	0.025,
	62000.00,	2500,	0.025,
	75000.00,	1850,	0.020,
};
size_t nhouses = sizeof (houses) / sizeof (struct houseinfo);

double
calctax (struct houseinfo *h)
{
	return (h->price / 1000) * h->tax_rate;
}

#define NYEARS 5

int
main()
{
	int house, year;
	double total_cost[nhouses];

	for (house = 0; house < nhouses; house++) {
		total_cost[house] = houses[house].price;

		for (year = 0; year < NYEARS; year++) {
			total_cost[house] += houses[house].heat_per_year;
			total_cost[house] += calctax (&houses[house]);

			printf ("After %d year(s), total cost of house %d "
			        "is %7.2f\n", year + 1, house + 1,
				total_cost[house]);
		}

		printf ("\n");
	}

	for (house = 0; house < nhouses; house++) {
		printf ("Cost of house %d is %7.2f\n", house + 1,
			total_cost[house]);
	}
}
