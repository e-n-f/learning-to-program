program foo (output);

var
	s: string;
	f: text;
	done: boolean;

begin
	reset (f, '.mail');
	done := false;
	repeat
		readln (f,s);
		if s='' then done := true;
		if substr (s,1,5)='From:' then begin
			write (substr (s, 7, 28));
			done := true
		end
	until done = true;
	close (f);
	write (' -- ');
        reset (f, '.mail');
        done := false;
        repeat
                readln (f,s);
                if s='' then done := true;
                if substr (s,1,8)='Subject:' then begin
                        write (substr (s, 10, 28));
        		done := true
		end
	until done = true;
	close (f);
	writeln
	

end.
