program discorporate;

	var
		outfile: text;
		infname: string;
		tempfname: string;
		outfname: string;
		x: integer;
		startWord, word: char;
		tempfile: text;


begin

	tempfname := oldfilename('blah');
	outfname := newfilename('Save story as', 'New story');
	rewrite(outfile, outfname);
	reset(tempfile, tempfname);
	x := 0;
	writeln('What letter should the story begin with?');
	read(startWord);
	writeln;
	writeln;
	repeat
		read(tempfile, word)
	until (eof(tempfile)) or (word = startWord);
	if eof(tempfile) then
		writeln('That letter is not there.Sorry')
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
				read(tempfile, word);
				if word = startWord then
					begin
						read(tempfile, StartWord);
						write(startword);
						write(outfile, startword);
						x := x + 1;
						if ((x > 50) and (word = ' ')) or (x = 70) then
							begin
								writeln;
								writeln(outfile);
								x := 0
							end
					end
			until 0 = 1
		end
end.