program saveScreen;


	const
		appleID = 128;
		fileID = 129;
		dummyID = 130;

		menuCount = 3;
		windowID = 128;

		appleM = 1;
		fileM = 2;
		dummyM = 3;

		xsize = 64;
		ysize = 42;


	type

		someBits = packed array[0..xsize, 0..ysize] of boolean;

	var
		i: integer;
		dragrect: rect;
{ourWIndow: windowptr;}
		whichWindow: windowptr;
		theChar: char;
		growSize: longint;

		blah: boolean;
		err: osErr;

		doneflag: boolean;
		myPort: grafPtr;
		myMenus: array[1..menuCount] of MenuHandle;
		myEvent: eventRecord;
		mouseLoc: point;
		mouseMoveRgn: rgnHandle;
		MoveRgn: rect;
		test: boolean;
		countdown: longint;


		myBits, newBits: someBits;

		x, y: integer;
		total: integer;

		firstTime: boolean;

		live: boolean;

		a, b, c, d, e, f, g, h: boolean;

		theRect: rect;
		thePoint: point;

		needFullUpdate: boolean;



	procedure setUpMenus;

		var
			i: integer;

	begin
		myMenus[appleM] := getmenu(appleID);
		addResmenu(mymenus[appleM], 'DRVR');
		mymenus[fileM] := getmenu(fileID);
		myMenus[dummyM] := getmenu(dummyID);

		for i := 1 to MenuCount do
			insertmenu(myMenus[i], 0);
		drawmenubar
	end;



	procedure doCommand (mResult: longint);

		var
			theItem: integer;
			theMenu: integer;
			name: str255;
			temp: integer;

	begin
{sysbeep(1);}
		theItem := loWord(mresult);
		theMenu := hiWord(mresult);
		case theMenu of
			appleID: 
				begin
					getItem(myMenus[appleM], theItem, name);
					if theitem <> 1 then
						begin
							temp := openDeskAcc(name);

						end
					else
						begin
							temp := alert(130, nil)
						end

				end;

			fileID: 
				case theItem of
					1: 

						doneFlag := true
				end

		end;
		Hilitemenu(0);
	end;




	function ya (i: boolean): integer;

	begin
		if i then
			ya := 1
		else
			ya := 0;
	end;


	procedure bar;

	begin
	end;


begin
	flushEvents(everyEvent, 0);
	initcursor;
	dragRect := screenBits.bounds;
{myPort := GrafPtr(NewPtr(sizeOf(grafPort)));}
{openport(myPort);}

	myPort := getNewWindow(128, nil, pointer(-1));

{    showWindow(myPort);}

	MouseMoveRgn := newRgn;

	setupMenus;
	setport(myPort);

{rewrite(f, 'test file');}

{find finder's id, make it current.}

	getMouse(mouseLoc);
	localToGlobal(mouseLoc);

	movergn.left := mouseLoc.h - 2;
	moveRgn.right := mouseLoc.h + 2;
	moveRgn.top := mouseLoc.v - 2;
	moveRgn.bottom := mouseLoc.v + 2;

	rectRgn(MouseMoveRgn, moveRgn);

	setPort(myPort);

	penPat(white);
{paintrect(0, 0, 40, 40);}

	penpat(black);
	paintRect(myPort^.portRect);
	moveto(10, 10);
{lineto(10, 10);}

	moveto(11, 11);
{lineto(11, 11);}

	moveto(11, 12);
{lineto(9, 12);}

	thePoint.h := 0;
	thePoint.v := 0;

	theRect.top := 0;
	theRect.left := 0;
	theRect.bottom := ysize;
	theRect.right := xsize;

{shieldCursor(theRect, thePoint);}


	for x := 0 to xsize do
		for y := 0 to ysize do
			begin
				myBits[x, y] := boolean(random mod 2);
				newBits[x, y] := myBits[x, y]
			end;

	showCursor;

	sysbeep(1);



	firstTime := true;
	repeat


		if waitNextEvent(everyEvent, myEvent, 1, nil) then
			begin
				case MyEvent.what of
					mouseDown: 
						case FindWindow(myEvent.where, whichWindow) of
							inSysWindow: 
								systemClick(myEvent, whichWindow);
							inDrag: 
								begin
									dragWindow(whichWindow, myEvent.where, dragRect);
								end;
							inMenuBar: 
								doCommand(menuSelect(myEvent.where));
							inContent: 
								begin
									if whichWindow <> frontWindow then
										begin
											selectWindow(whichWindow);
										end
									else
										begin
										end;
								end;
						end;
					keyDown, autoKey: 
						begin
							theChar := chr(bitAnd(myEvent.message, charCodeMask));
							if bitAnd(myEvent.modifiers, cmdKey) <> 0 then
								doCommand(menuKey(theChar));
						end;
					updateEvt: 
						begin
							beginUpdate(windowPtr(myEvent.message));
							needFullUpdate := true;
							endupdate(windowPtr(myEvent.message));
						end;

				end;
			end
		else
			begin
{countdown := countdown + 1;}

				setPort(myPort);

				if needFullUpdate then
					begin
						penpat(black);
						paintRect(myPort^.portRect);
					end;




				for y := 1 to ysize - 1 do{screenBits.bounds.bottom do}
					begin
						for x := 1 to xsize - 1 do {screenBits.bounds.right do}
							begin

{if myBits [ x , y ] then if myBits[x-1,y-1] ;}

								total := 0;
								if myBits[x - 1, y - 1] then
									total := 1;
								if myBits[x, y - 1] then
									total := succ(total);
								if myBits[x + 1, y - 1] then
									total := succ(total);

								if myBits[x - 1, y] then
									total := succ(total);
								if myBits[x + 1, y] then
									total := succ(total);

								if myBits[x - 1, y + 1] then
									total := succ(total);
								if myBits[x, y + 1] then
									total := succ(total);
								if myBits[x + 1, y + 1] then
									total := succ(total);

{total := ya(myBits[x - 1, y - 1]) + ya(myBits[x, y - 1]) + ya(myBits[x + 1, y - 1]);}
{total := total + ya(myBits[x - 1, y]) + ya(myBits[x + 1, y]);}
{total := total + ya(myBits[x - 1, y + 1]) + ya(myBits[x, y + 1]) + ya(myBits[x + 1, y + 1]);}

								live := false;
								if (total = 3) then
									live := true;

								if (total = 2) and myBits[x, y] then
									live := true;

								newBits[x, y] := live;


							end
					end;


				for y := 0 to ysize do{screenBits.bounds.bottom do}
					begin
						for x := 0 to xsize do {screenBits.bounds.right do}
							begin


								if (newBits[x, y] <> myBits[x, y]) or (needFullUpdate and newBits[x, y]) or (firstTime and newBits[x, y]) then
									begin
										if newBits[x, y] then
											PenPat(white)
										else
											penPat(black);
										PaintRect(y + y + y + y + y + y + y + y, x + x + x + x + x + x + x + x, y + y + y + y + y + y + y + y + 7, x + x + x + x + x + x + x + x + 7);

									end
							end;
					end;

				firstTime := false;

				myBits := newBits;
				needFullUpdate := false;
			end

	until doneFlag = true

end.