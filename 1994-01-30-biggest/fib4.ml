(* calc the fibs with tail recursion

   eric fischer
   1 feb 1994

   almost equidistant to noon and midnight.  argh.
*)

fun fibby (first, second, 0) = second
 |  fibby (first:int, second:int, count) =
      fibby (second, first + second, count - 1);

fun fib(x) = fibby (0,1,x);

use "printing.ml";
