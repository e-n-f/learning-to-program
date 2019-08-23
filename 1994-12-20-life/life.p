program life(input, output);

const
	xsize = 78;
	ysize = 42;

type
	someBits = packed array [0..xsize, 0..ysize] of boolean;

var
	myBits, newBits: someBits;
	x, y, total: integer;
	s: string;
	done: boolean;

procedure refresh;
begin
	write ('');
	for y := 0 to ysize div 2 do begin
		for x := 0 to xsize do begin
			if myBits[x,y*2]
				then if myBits[x, y*2+1]
					then write ('8')
					else write (chr(176))
				else if myBits[x, y*2+1]
					then write ('o')
					else write (' ');
					 
		end;
		writeln
	end
end;

function ya (xx: integer; yy: integer): integer;

begin
	if myBits[x+xx, y+yy] then ya := 1 else ya := 0
end;

begin
	for x := 0 to xsize do for y := 0 to ysize do myBits[x,y] := false;

	myBits[10,10] := true;
	myBits[11,11] := true;
	myBits[11,12] := true;
	myBits[10,12] := true;
	myBits[9,12] := true;

	refresh;
	done := false;

	repeat
		for x := 1 to xsize - 1 do begin
			for y := 1 to ysize - 1 do begin
				total:=ya(-1,-1);
				total := total + ya(0,-1);
				total := total + ya(1,-1);
				total := total + ya(-1,0);
				total := total + ya(1,0);
				total := total + ya(-1,1);
				total := total + ya(0,1);
				total := total + ya(1,1);

				newBits[x,y] := false;
				if total = 3 then newBits[x,y] := true;
				if (total=2) and myBits[x,y]
					then newBits[x,y] := true
			end
		end;

		myBits := newBits;
		refresh;

		readln (s);
		if s = 'q' then done := true
	until done = true
end.
