program discorporate;

	var
		outfile: text;
		infname: string;
		tempfname: string;
		outfname: string;
		x: integer;
		startWord, word: string;
		tempfile: text;

	procedure tabulate (inName: string; tempName: string);

		var
			inputfile: text;
			c: char;

	begin
		reset(inputfile, inName);
		rewrite(tempfile, tempName);
		read(inputfile, c);
		repeat

			if (c in ['a'..'z', 'A'..'Z', ',', '.', '''', '"']) then
				begin
					write(tempfile, c);
					write(c);
					read(inputfile, c)
				end
			else
				begin
					writeln(tempfile);
					writeln;
					repeat
						read(inputfile, c);
						if (eoln(inputfile)) or (eof(inputfile)) then
							readln(inputfile)
					until (eof(inputfile)) or (c in ['a'..'z', 'A'..'Z', ',', '.', '''', '"'])
				end;
{if eoln(inputfile) then}
{begin}
{readln ( inputfile );}
{writeln ( tempfile );}
{writeln}
{end}
		until eof(inputfile);
		close(inputfile);
		close(tempfile)
	end;

begin

	infname := oldfilename('blah');
	if infname <> '' then
		begin
			tempfname := newfilename('Temporary file', 'Temporary');
			writeln('Tabulating contents of sample file.');
			tabulate(infname, tempfname)
		end
	else
		begin
			tempfname := oldfilename('blah');
			writeln('Using previously made file.')
		end;

	outfname := newfilename('Save story as', 'New story');
	rewrite(outfile, outfname);
	reset(tempfile, tempfname);
	x := 0;
	writeln('What word should the story begin with?');
	readln(startWord);
	repeat
		readln(tempfile, word)
	until (eof(tempfile)) or (word = startWord);
	if eof(tempfile) then
		writeln('That word is not there.Sorry')
	else
		begin
			write(word, ' ');
			write(outfile, word, ' ');
			startword := word;
			repeat
				if eof(tempfile) then
					begin
						close(tempfile);
						reset(tempfile, tempfname)
					end;
				readln(tempfile, word);
				if word = startWord then
					begin
						readln(tempfile, StartWord);
						write(startword, ' ');
						write(outfile, startword, ' ');
						x := x + 1;
						if x = 6 then
							begin
								writeln;
								writeln(outfile);
								x := 0
							end
					end
			until 0 = 1
		end
end.