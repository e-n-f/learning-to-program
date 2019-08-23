program Mail;

{ Insert your declarations here }

	var
		f1: text;
		f2: text;
		s: string;
		f1name: string;
		f2name: string;
		a: integer;

begin
	f1name := oldfilename('blah');
	f2name := newfilename('Save the converted file as:', 'Untitled mail');
	reset(f1, f1name);
	rewrite(f2, f2name);
	repeat
		readln(f1, s);
		a := pos('0000000', s);
		if a = 0 then
			begin
				writeln(s);
				writeln(f2, s)
			end
		else
			begin
				write(f2, 'From ???@??? ');
				writeln(f2, s);
				writeln('-----------');
				writeln(s)
			end
	until eof(f1)
end.