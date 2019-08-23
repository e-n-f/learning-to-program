/* 3.14.c

   eric fischer
   enf1

   more like 2:30 now
*/

#include <stdio.h>
#include <stdlib.h>

void swap (int *, int *);

void doPartition (int m, int n, int *A, int v) {
  int i, j;
  char notDone = 1;

  /* m is the start of the partition
     i is the last element which is less than v
     j is the first element that's greater than v
     n is the last element of the partition.
     A is the array of ints we're trying to sort.
     v is the value by which we partition things.
  */

  i = m;
  j = n;

  while (notDone) {
    while (A[i] < v) i++;
    while (A[j] > v) j--;
    if (i >= j) notDone = 0;
    if (notDone) swap (&(A[i]), &(A[j]));
  }

  printf ("i = %d\nj = %d\n", i, j);
}

void getThing (int *theVar) {
  scanf ("%d", theVar);
}

void swap (int *first, int *second) {
  int temp;

  temp = *first;
  *first = *second;
  *second = temp;
}

int main (void) {
  int theArray[100];  /* yeah, I know I should allocate this dynamically,
                         but it's just an example, right? */
  int m, n, v;
  int count;

  printf ("Lower bound of array? (m): ");
  getThing (&m);
  printf ("Upper bound of array? (n): ");
  getThing (&n);

  printf ("\n");
  for (count = m; count <= n; count++) {
    printf ("A[%d] = ", count);
    getThing (&(theArray[count]));
  }
  printf ("\n");

  printf ("Partition point? (v): ");
  getThing (&v);

  doPartition (m, n, theArray, v);

  for (count = m; count <= n; count++) {
    printf ("A[%d] = %d\n", count, theArray[count]);
  }
}
