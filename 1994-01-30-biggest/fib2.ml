(* create an array to hold the fibs, then fill it.

   eric fischer
   1 feb 1994

   much closer to noon than midnight.
*)

open Array;
val fibs = array(100,0);

fun fib(0) = (update (fibs,0,1);
              1)
 |  fib(1) = (update (fibs,1,1);
              1)
 |  fib(x) = if (sub (fibs,x)) <> 0
             then sub (fibs,x)
             else (update (fibs, x, fib (x-1) + fib (x-2));
                   sub(fibs,x));

use "printing.ml";
