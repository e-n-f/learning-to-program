program Works2Word;

	var
		inf: text;
		outf: text;
		inname: string;
		outname: string;
		cc: char;
		c: integer;
		c1: char;
		c2: char;
		c3: char;
		c4: char;
		loop: integer;
		cr: boolean;
		l: integer;
		n: integer;


begin
	inname := OldFileName('flarn');
	outname := NewFileName('Save translated text asÉ', 'Untitled MS Word');
	reset(inf, inname);
	rewrite(outf, outname);
	for loop := 0 to 299 do
		read(inf, cc);
	writeln(outf, '{\rtf1\mac\deff2 {\fonttbl{\f2\froman New York;}{\f3\fswiss Geneva;}{\f4\fmodern Monaco;}{\f5\fscript Venice;}{\f6\fdecor London;}{\f7\fdecor Athens;}{\f8\fdecor San Francisco;}{\f10\fdecor TeleCom;}{\f11\fnil Cairo;}{\f12\fnil Los Angeles;}');
	writeln(outf, '{\f16\fnil Palatino;}{\f18\fnil Zapf Chancery;}{\f20\froman Times;}{\f21\fswiss Helvetica;}{\f22\fmodern Courier;}{\f23\ftech Symbol;}{\f33\fnil Avant Garde;}{\f130\fnil Thin Telecom;}{\f200\fnil JacksonvilleOldStyle;}{\f234\fnil Bookman;}');
	writeln(outf, '{\f1024\fnil Thin Geneva;}{\f1025\fnil Thomas;}{\f1279\fnil Albatross;}{\f1462\fnil Helvetica Narrow;}{\f2219\fnil StymieLight;}{\f3017\fnil GoudyHundred;}}{\colortbl\red0\green0\blue0;\red0\green0\blue255;\red0\green255\blue255;\red0\green255\blue0;');
	writeln(outf, '\red255\green0\blue255;\red255\green0\blue0;\red255\green255\blue0;\red255\green255\blue255;}{\stylesheet{\f16 \sbasedon222\snext0 Normal;}}\widowctrl\ftnbj \sectd \linemod0\linex0\cols1\endnhere \pard\plain \f16 ');
	write(outf, '{\f22 ');
	while not (eof(inf)) do
		begin
			read(inf, c1);
			read(inf, c2);
			if ord(c2) = 208 then
				writeln(outf, ' \par ');
			if ord(c2) = 219 then
				writeln(outf, '}{\f22 \fs', trunc(24 / ord(c1) * 10), ' ');
			if ord(c2) = 220 then
				writeln(outf, '}{\f20 \fs24 ');
			if ord(c2) = 221 then
				writeln(outf, '}{\f20 \fs20 ');
			if ord(c2) = 223 then
				writeln(outf, '\pard \qj ');
			if ord(c2) = 224 then
				writeln(outf, '\pard ');
			if ord(c2) = 225 then
				writeln(outf, '\pard \qc ');
			if ord(c2) = 0 then
				begin
					read(inf, c3);
					read(inf, c4);
					l := ord(c4);
					cr := false;
					if l > 127 then
						begin
							cr := true;
							l := l - 128
						end;
					for n := 1 to l do
						begin
							read(inf, cc);
							c := ord(cc);
							if cc = '{' then
								write(outf, '\{')
							else if cc = '}' then
								write(outf, '\}')
							else if cc = '\' then
								write(outf, '\\')
							else
								begin
									if c > 31 then
										write(outf, c);
									if c = 1 then
										write(outf, '{\b ');
									if (c = 2) or (c = 4) or (c = 6) or (c = 8) then
										write(outf, '}');
									if c = 3 then
										write(outf, '{\up6 ');
									if c = 5 then
										write(outf, '{\dn6 ');
									if c = 7 then
										write(outf, '{\u ')
								end
						end;
					if cr then
						writeln(outf, '\par ')
				end;
			writeln(outf, '}');
			close(outf);
			close(inf)
		end
end.