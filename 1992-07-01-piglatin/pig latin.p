{ Eric's Wretched Pig Latin Translator.  Copyright 1992, but who cares? }

program PigLatin;

	var
		w: array[1..20] of char;
		c: char;
		count: integer;
		f: string;
		g: string;
		a: text;
		b: text;
		cc: char;

	procedure Printword;

		var
			count2: integer;
			n: integer;

	begin
		if (count = 0) then
			write(b, c)
		else
			begin
				count2 := 1;
				while (count2 < count) and not ((w[count2] in ['A', 'E', 'I', 'O', 'U', 'a', 'e', 'i', 'o', 'u'])) do
					count2 := count2 + 1;
				if (w[1] in ['A'..'Z']) and (w[count2] in ['a'..'z']) then
					begin
						w[1] := chr(ord(w[1]) + 32);
						w[count2] := chr(ord(w[count2]) - 32)
					end;
				for n := count2 to count do
					write(b, w[n]);
				for n := 1 to count2 - 1 do
					write(b, w[n]);
				write(b, 'ay');
				write(b, c)
			end;
		write(c);
		if not (eof(a)) then
			if eoln(a) then
				begin
					writeln(b);
					writeln
				end;
		count := 0
	end;


begin
	showtext;
	writeln('Welcome to this wretched pig latin program.  Please provide a file name.');
	f := OldFileName('flarn');
{g := concat(f, '.pl');}
	g := NewFileName('Save translated text asÉ', 'Untitled Pig Latin');
	open(a, f);
	open(b, g);
	reset(a);
	rewrite(b);
	writeln;
	writeln;

	count := 0;
	while ((not (eof(a))) and (button = false)) do
		begin
			read(a, c);
			if (((c < 'A') or ((c > 'Z') and (c < 'a')) or (c > 'z')) and (c <> '''')) then
				printword
			else
				begin
					write(c);
					count := count + 1;
					w[count] := c
				end
		end;
	close(a);
	close(b)
end.