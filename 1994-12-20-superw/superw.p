program superw (input, output);

var 
	f1, f2: text;
	s1, s2: string;
	done: boolean;

begin
	reset (f1, '.w');
	repeat
		readln (f1, s1);
		reset (f2, '.finger');
		done := false;
		repeat
			readln (f2, s2);
			if substr (s1, 1, 8) = substr (s2, 1, 8) then begin
				write (substr (s2, 1, 30));
				write ('  ');
				write (substr (s1, 20, 13));
				write ('  ');
				writeln (substr (s1, 48, 32));
				done := true
			end
		until (done) or (eof (f2));
		close (f2);
		if (not (done)) then writeln (s1)
	until eof (f1);
	close (f1)
end.