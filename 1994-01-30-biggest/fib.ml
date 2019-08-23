(* ordinary, recursive fib calculations.

   exciting, eh?

   eric fischer
   enf1
*)

fun fib(x) =
  if x = 0 then 1
  else if x = 1 then 1
       else fib(x-1) + fib(x-2);

use "printing.ml";

(* printing.ml

   eric fischer
   enf1

   1 feb

   support functions for printing lots of fibs in one fell swoop
   instead of having to call fib over and over again manually.
*)

fun for (i,j) f = if i<=j then (f i; for(i+1,j) f) else ()

fun printone(which) =
  (print ("fib of ");
   print (which);
   print (" is ");
   print (fib(which));
   print ("\n")
 );

fun printfibs(blah) =
  for (0, blah) printone;

