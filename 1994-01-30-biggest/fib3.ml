(* calc the fibs iteratively.

   eric fischer
   1 feb 1994

   much closer to noon than midnight, but not as much closer as before.
*)


(* bleah.  these shouldn't be globals but i don't know ml well enough... *)

val first = ref 0;
val second = ref 0;
val count = ref 0;

fun fib(x) = (
  first := 0;
  second := 1;
  count := x;

  while (!count > 0) do (
    second := !first + !second;
    first := !second - !first;
    count := !count - 1
  );
  !second
);
