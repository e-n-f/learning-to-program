
program Sumpyville;

	var
		wind1, wind2: windowPtr;
		r, r2: rect;
		b, b2: bitMap;
		wid: longInt;
		y: integer;
		fNum: integer;
		sz: integer;
		fName, fStyle: string;
		fSize: integer;
		done: boolean;
		theString: string;
		theRect: rect;

	procedure demo;
	begin
		setport(wind2);
		textSize(48);
		moveto(0, 36 + 3);
		textFont(times);
{textface([italic]);}
		drawString('etaoinshrdlu');
		wid := (((stringWidth('etaoinshdrlu')) div 4) * 4) + 4;

		penMode(srcCopy);
		setrect(r, 0, 0, wid + 3, 48 + 3);
		setrect(r2, 0, 0, (wid div 4), 12);

		b := wind2^.portBits;
		b2 := wind1^.portBits;

		copyBits(b, b2, r, r2, ditherCopy, nil);

		setPort(wind1);
		moveto(1, 19);
		textMode(srcCopy);
		textFont(times);
		textSize(12);
{textface([italic]);}
		drawString('etaoinshrdlu');
{drawChar('&');}

	end;

	procedure MyCopyBits (rect2, rect1: rect; port2, port1: grafPtr);

		var
			x, y: integer;
			x1, y1: integer;
			red: longInt;
			green: longInt;
			blue: longInt;
			col: RGBColor;
			x2, y2: integer;

	begin
		for x := (rect1.topLeft.h) to (rect1.botRight.h) do
			for y := rect1.topLeft.v to rect1.botRight.v do
				begin
					red := 0;
					green := 0;
					blue := 0;
					for x1 := 0 to 3 do
						for y1 := 0 to 3 do
							begin
								x2 := ((x - rect1.topLeft.h) * 4 + x1) + rect2.topLeft.h;
								y2 := ((y - rect1.topLeft.v) * 4 + y1) + rect2.topLeft.v;
								setPort(port2);
								getCPixel(x2, y2, col);
								red := red + col.red;
								green := green + col.green;
								blue := blue + col.blue;
							end;
					setPort(port1);
					col.red := red div 16;
					col.blue := blue div 16;
					col.green := green div 16;
					setCPixel(x, y, col);
				end;

	end;

	procedure realDrawChar (c: char);

	begin
		drawChar(c);
	end;



	procedure myDrawChar (c: char);

		var
			curPort: grafPtr;
			tempPort: grafPtr;

			startPt: point;
			txSize: integer;
			txFont: integer;
			txFace: Style;
			txMode: integer;

			charWid: integer;

			ascent, descent: integer;

			sourceRect, playRect: rect;

			cw, bigCw: integer;

	begin

		getPort(curPort);
		tempPort := wind2;

		startPt := curPort^.pnLoc;
		txSize := curPort^.txSize;
		txFace := curPort^.txFace;
		txMode := curPort^.txMode;
		txFont := curPort^.txFont;

		if txSize > 36 then
			RealDrawChar(c)
		else
			begin
				charWid := charWidth(c) + 1;
				cw := (charWid - 1) * 4;

				ascent := txSize;
				descent := txSize div 2;

				sourceRect.topLeft.h := startPt.h - 1;
				sourceRect.topLeft.v := startPt.v - ascent;
				sourceRect.botRight.h := startPt.h + charWid + 2;
				sourceRect.botRight.v := startPt.v + descent;

				playRect.topLeft.h := 2;
				playRect.topLeft.v := (txSize * 4) - (ascent * 4);
				playRect.botRight.h := (charWid * 4) + 4 + 8;
				playRect.botRight.v := (txSize * 4) + (descent * 4);

				setPort(tempPort);

				copyBits(curPort^.portBits, tempPort^.portBits, sourceRect, playRect, srcCopy, nil);



				setPort(curPort);
				RealDrawChar(c);

				setPort(tempPort);
				textSize(txSize * 4);
				textFace(txFace);
				textMode(txMode);
				textFont(txFont);
				bigCw := charWidth(c);

				moveto(4 + ((cw - bigCw) div 2), (txSize * 4) - 1);

				RealDrawChar(c);
{frameRect(playRect);}


				setPort(curPort);

				copyBits(tempPort^.portBits, curPort^.portBits, playRect, sourceRect, ditherCopy, nil);

{myCopyBits(playRect, sourceRect, tempPort, curPort);}
			end
	end;

	procedure myDrawString (s: str255);

		var
			i: integer;
	begin
		for i := 1 to length(s) do
			myDrawChar(s[i]);
	end;


begin
	setRect(theRect, 0, 400, screenbits.bounds.right, screenbits.bounds.bottom);

	settextrect(theRect);
	showtext;




	wind1 := getNewCWindow(128, nil, pointer(-1));
	wind2 := getNewCWindow(129, nil, pointer(-1));


	setPort(wind1);
	done := false;

{setFractEnable(true);}

	if false then
		for y := 0 to 60 do
			begin
				if y mod 2 = 0 then
					foreColor(redColor)
				else
					foreColor(blueColor);
				moveto(0, y);
				line(50, 0);
			end;


	foreColor(blackColor);

	y := 0;

	repeat
		write('Font?  ');
		readln(fName);
		if fName = '' then
			done := true
		else
			begin
				write('Size?  ');
				readln(fSize);

				write('Style? ');
				readln(fStyle);

				getFNum(fName, fNum);
				textFont(fNum);
				textSize(fSize);
				textface([]);

				if fStyle = 'italic' then
					textface([italic]);
				if fStyle = 'boldItalic' then
					textface([bold, italic]);
				if fStyle = 'bold' then
					textface([bold]);

				y := y + fSize;

				write('Text?  ');
				readln(theString);
				writeln;

				setPort(wind1);

				moveto(5, y);
				drawString(theString);

				y := y + fSize;
				moveto(5, y);
				myDrawString(theString);

			end
{myDrawString('Jackdaws love my big sphinx of quartz');}

	until done

end.