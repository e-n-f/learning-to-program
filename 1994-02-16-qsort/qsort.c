/* 3.13.c

   eric fischer
   enf1

   about 2:00 am, 16 feb

   in think c because i really don't want to write this at 2400bps
*/

#include <stdio.h>
#include <stdlib.h>

void swap (int *, int *);

void doPartition (int m, int n, int *A, int v) {
  int i, k;

  /* m is the start of the partition
     i is the last element which is less than v
     k is the first element we don't know anything about
     n is the last element of the partition.
     A is the array of ints we're trying to sort.
     v is the value by which we partition things.
  */

  i = m-1;
  k = i+1;

  while (k < n) {
    k++;
    if (A[k] < v) {
      swap (&(A[k]), &(A[i+1]));
      i++;
    }
  }

  printf ("i = %d\nk = %d\n", i, k);
}
