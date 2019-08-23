program Fooble;

	var
		c: char;
		s: string;
		f1: text;
		f2: text;
		done: boolean;
		f1name: string;
		f2name: string;
		col: integer;
		lin: integer;

	procedure tab (coln: integer);

		var
			i: integer;
	begin
		for i := col to coln - 1 do
			write(f2, ' ');
		col := coln;
	end;

begin
	f1name := oldFileName('');
	write('Device for output?   ');
	f2name := 'printer:';

	reset(f1, f1name);
	rewrite(f2, f2name);

	c := chr(27);
{write(f2, c, 'Q');}

	done := false;
	col := 1;
	lin := 1;
	repeat
		read(f1, c);
		s := c;
		if (ord(c) < 32) or (ord(c) > 127) then
			s := '...';
		if c = 'Þ' then
			s := 'fi';
		if c = 'ß' then
			s := 'fl';
		if c = 'Ô' then
			s := '''';
		if c = 'Õ' then
			s := '''';
		if c = 'Ò' then
			s := '"';
		if c = 'Ó' then
			s := '"';
		if ord(c) = 9 then
			begin
				tab(((col + 6) div 5) * 5 + 1);
				s := '';
			end;
		if eof(f1) then
			done := true;
		write(f2, s);
		col := col + length(s);
		if eoln(f1) then
			begin
				readln(f1);
				writeln(f2);
				col := 1;
				lin := lin + 1;
				close(f2);
				rewrite(f2, f2name);
				if eoln(f1) then
					begin
						readln(f1);
						writeln(f2);
						lin := lin + 1;
					end;
			end;
		if lin > 60 then
			begin
				lin := 1;
				c := chr(12);
				write(f2, c);
			end;

	until done

end.