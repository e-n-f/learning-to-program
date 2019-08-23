program chars (output);

var x,y: integer;

begin
	for x := 2 to 15 do begin
		for y := 0 to 15 do write (char (16*x+y));
		writeln
	end
end.
