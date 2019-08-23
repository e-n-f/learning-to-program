program discorporate;

	var
		outfile: text;
		infname: string;
		tempfname: string;
		outfname: string;
		x: integer;
		startWord, word, word2, StartWord2: char;
		tempfile: text;


begin

	tempfname := oldfilename('blah');
	outfname := newfilename('Save story as', 'New story');
	rewrite(outfile, outfname);
	reset(tempfile, tempfname);
	x := 0;
	writeln('What two letters should the story begin with?');
	read(startWord);
	read(startWord2);
	writeln;
	writeln;
	repeat

		read(tempfile, word);
		read(tempfile, word2)
	until (eof(tempfile)) or ((word = startWord) and (word2 = startword2));
	if eof(tempfile) then
		writeln('That letter is not there.  Sorry')
	else
		begin
			write(word);
			write(outfile, word);
			write(word2);
			write(outfile, word2);
			startword := word;
			word := word2;
			repeat
				word := word2;
				if eof(tempfile) then
					begin
						close(tempfile);
						reset(tempfile, tempfname)
					end;
				read(tempfile, word2);
				if (word = startWord) and (word2 = startword2) then
					begin
						startword := startword2;
						read(tempfile, StartWord2);
						write(startword2);
						write(outfile, startword2);
						x := x + 1;
						if ((x > 50) and (startword2 = ' ')) or (x = 70) then
							begin
								writeln;
								writeln(outfile);
								x := 0
							end
					end
			until 0 = 1
		end
end.