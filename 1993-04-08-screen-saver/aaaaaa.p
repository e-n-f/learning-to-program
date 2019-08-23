program saveScreen;

	uses
		Processes;

	const
		appleID = 128;
		fileID = 129;
		dummyID = 130;

		menuCount = 3;
		windowID = 128;

		appleM = 1;
		fileM = 2;
		dummyM = 3;


	type
		clockInfo = record
				analog: boolean;
				digital: boolean;
				date: (none, long, short, numerical);
				active: boolean;
				wind: windowPtr;
				size: point
			end;

	var
		i: integer;
		dragrect: rect;
		ourWIndow: windowptr;
		whichWindow: windowptr;
		theChar: char;
		currenttime: dateTimeRec;
		prevTime: dateTimeRec;
		growSize: longint;
		ticks: array[1..12] of point;
		f, f2: text;
		lof: longInt;

		blah: boolean;
		err: osErr;

		doneflag: boolean;
		myPort: grafPtr;
		time2sleep: boolean;
		myMenus: array[1..menuCount] of MenuHandle;
		myEvent: eventRecord;
		mouseLoc: point;
		mouseMoveRgn: rgnHandle;
		MoveRgn: rect;
		test: boolean;
		countdown: longint;
		ourProcess, frontProcess: processSerialNumber;

		signature: osType;
		process: processSerialNumber;
		infoRec: processInfoRec;
		aFSSpecPtr: fsSpecPtr;
		countUp: integer;
		lastThing, lastTime, lastQuote: longInt;

		showTime: boolean;
		showQuotes: boolean;

		lastTimeLoc: point;

	function findAProcess (signature: OSType; var Process: processSerialNumber; var InfoRec: processInfoRec; aFSSpecPtr: fsSpecPtr): boolean;
 {from Inside Mac 6, p 29.11}

	begin
		findAProcess := false;
		process.HighLongOfPsn := 0;
		process.LowLongOfPsn := kNoProcess;

		infoRec.processInfoLength := sizeOf(processInfoRec);
		infoRec.processName := stringPtr(newPtr(32));
		infoRec.processAppSpec := nil;

		while (getNextProcess(process) = noErr) do
			begin
				if getProcessInformation(process, InfoRec) = noErr then
					begin
						if (InfoRec.processSignature = signature) then
							begin
								findAProcess := true;
								exit(findAProcess);
							end;
					end;
			end;

	end;


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
							setPort(ourWindow)
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

	procedure getQuote (var s: str255);
		var
			i, j: longInt;

	begin
		i := ((tickCount) mod (Lof - 40)) + 1;
		seek(f, i);
		if eof(f) then
			seek(f, 1);
		readln(f, s);
		if eof(f) then
			seek(f, 1);
		readln(f, s);

{i := abs(random + tickCount) mod 70;}
{for j := 1 to i + 20 do}
{begin}
{if eof(f) then}
{    reset(f);}
{    readln(f, s);}
{    end;}
	end;

	procedure split (var s, t: str255; i: integer);

		var
			j: integer;
			s1: str255;

	begin
		s1 := s;
		if (s = '') or (s = ' ') then
			begin
				s := '';
				t := '';
			end
		else
			begin
				for j := 1 to i do
					begin
						if s1[j] = ' ' then
							begin
								s := copy(s1, 1, j);
								t := copy(s1, j + 1, length(s1) - j);
							end
					end
			end

	end;

	function getLof: longInt;

	begin
		seek(f, maxlongint);
		getLof := filepos(f);

	end;

	function Max (x1, x2: longInt): longInt;
	begin
		if x1 > x2 then
			max := x1
		else
			max := x2;
	end;


	procedure doScreenSave;

		var
			DoneFlag: boolean;
			blah: boolean;
			longTime: str255;
			lastTimeS: str255;
			timeInt: longInt;
			i: integer;
			thisTimeLoc: point;
			QuoteS, qs2, qs3, qs4: str255;
			y1: integer;
			x1, maxWid: longInt;


	begin


{setMBarHeight(i);}


		open(f, 'Quotes');
		lof := getLof;

		getMouse(mouseLoc);
		localToGlobal(mouseLoc);

		movergn.left := mouseLoc.h - 2;
		moveRgn.right := mouseLoc.h + 2;
		moveRgn.top := mouseLoc.v - 2;
		moveRgn.bottom := mouseLoc.v + 2;

		rectRgn(MouseMoveRgn, moveRgn);

		err := getFrontProcess(frontProcess);

		err := setFrontProcess(ourProcess);
		sizeWindow(ourWindow, myPort^.portRect.botRight.h, myPort^.portRect.botRight.v, true);
		showWindow(ourWindow);

		for i := 1 to 10 do
			begin
				blah := waitNextEvent(everyEvent, myEvent, 2, nil);
				if blah then
					case MyEvent.what of
						mouseDown: 
							begin
								case FindWindow(myEvent.where, whichWindow) of
									inSysWindow: 
										SystemClick(myEvent, whichWindow);
									inContent: 
										selectWindow(whichWindow);
								end;
							end;
						updateEvt: 
							begin
								beginUpdate(windowPtr(myEvent.message));
								endupdate(windowPtr(myEvent.message));
							end
					end;
			end;

		hideCursor;
		setPort(myPort);
		penpat(black);
		paintRect(myPort^.portRect);

		obscureCursor;
		lastThing := tickCount;
		doneFlag := false;
		lastTimeS := '';

		repeat

			if tickCount - lastThing > 3600 then
				begin
					setPort(myPort);
					penpat(black);
					paintRect(myPort^.portRect);

					obscureCursor;
					lastThing := tickCount;
					lastTimeS := '';
				end;
			if tickCount - lastQuote > 900 then
				begin
					textface([italic, condense]);
					getQuote(quoteS);
					setPort(myPort);
					penpat(black);
					paintRect(myPort^.portRect);

					obscureCursor;
					lastQuote := tickCount;
					lastThing := tickCount;
					lastTimeS := '';

					textmode(srcXor);
					textfont(20);
					textSize(36);
					y1 := 36 + abs(longint(random)) mod longint(myPort^.portRect.botRight.v - 144);

					split(quoteS, qs2, 25);
					split(qs2, qs3, 25);
					split(qs3, qs4, 25);
					maxWid := max(max(StringWidth(quoteS), stringWidth(qs2)), max(stringWidth(qs3), stringWidth(qs4)));

					x1 := abs(longint(random)) mod longint(myPort^.portRect.botRight.h - maxWid);

					moveto(x1, y1);
					drawString(quoteS);
					moveto(x1, y1 + 36);
					drawString(qs2);
					moveto(x1, y1 + 72);
					drawString(qs3);
					moveto(x1, y1 + 108);
					drawString(qs4);
				end;
			if tickCount - lastTime > 60 then
				begin
					textface([]);
					moveto(lastTimeLoc.h, lastTimeLoc.v);
					textmode(srcXor);
					textfont(20);
					textSize(36);
					getDateTime(timeInt);
					IUTimeString(timeInt, true, longTime);
					drawString(lastTimeS);
					lastTimeLoc.h := abs(longint(random)) mod longint(myPort^.portRect.botRight.h - stringWidth(longTime));
					lastTimeLoc.v := 36 + abs(longint(random)) mod longint(myPort^.portRect.botRight.v - 36);

					moveto(lastTimeLoc.h, lastTimeLoc.v);

					drawString(longTime);
					lastTimeS := longTime;
					lastThing := tickCount;
					lastTime := tickCount;
				end;

			if waitNextEvent(everyEvent, myEvent, 30, nil) then
				begin
					case MyEvent.what of
						mouseDown: 
							begin
								doneFlag := true;
								case FindWindow(myEvent.where, whichWindow) of
									inSysWindow: 
										SystemClick(myEvent, whichWindow);
									inMenuBar: 
										doCommand(menuSelect(myEvent.where));
									inContent: 
										selectWindow(whichWindow);
								end;
							end;

						keyDown, autoKey: 
							doneFlag := true;

						activateEvt: 
							begin
								if BitAnd(myEvent.modifiers, activeFlag) <> 0 then
									begin
									end
								else
									begin
										doneFlag := true;
									end
							end;
						updateEvt: 
							begin
								beginUpdate(windowPtr(myEvent.message));
								endupdate(windowPtr(myEvent.message));

{updateClock(windowPtr(myEvent.message));}
							end
					end;
				end;

			getmouse(mouseLoc);
			if not (ptinRgn(mouseLoc, mousemoveRgn)) then
				begin
					doneFlag := true;
				end;

{setMBarHeight(i);}

		until doneFlag = true;
		showCursor;
		hideWindow(ourWindow);

		err := setFrontProcess(frontProcess);
		showCursor;
		repeat
			blah := waitNextEvent(everyEvent, myEvent, 60, nil);
			err := getFrontProcess(frontProcess);
			err := sameProcess(frontProcess, ourProcess, blah);
		until blah = false;
		showCursor;
		close(f);
	end;


begin
	countUp := 0;
	flushEvents(everyEvent, 0);
	initcursor;
	myPort := GrafPtr(NewPtr(sizeOf(grafPort)));
	openport(myPort);
	MouseMoveRgn := newRgn;
	showTime := true;
	showQuotes := true;

	setupMenus;
	countdown := 0;
	setport(myPort);
	ourWindow := getNewWindow(128, nil, pointer(-1));
	ourWindow^.bkPat := black;

{rewrite(f, 'test file');}

{find finder's id, make it current.}

	blah := findAProcess('MACS', frontProcess, InfoRec, aFSSpecPtr);
	err := getCurrentProcess(ourProcess);
	err := setFrontProcess(frontProcess);
	getMouse(mouseLoc);
	localToGlobal(mouseLoc);

	movergn.left := mouseLoc.h - 2;
	moveRgn.right := mouseLoc.h + 2;
	moveRgn.top := mouseLoc.v - 2;
	moveRgn.bottom := mouseLoc.v + 2;

	rectRgn(MouseMoveRgn, moveRgn);

	repeat

		if (mouseLoc.h < 10) and (mouseLoc.v < 10) then
			countdown := maxInt;


		if waitNextEvent(everyEvent, myEvent, 60, nil) then
			begin
				case MyEvent.what of
					mouseDown: 
						case FindWindow(myEvent.where, whichWindow) of
							inSysWindow: 
								systemClick(myEvent, whichWindow);

							inMenuBar: 
								doCommand(menuSelect(myEvent.where));
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
							endupdate(windowPtr(myEvent.message));
						end;

				end;
			end;
{countdown := countdown + 1;}

		getMouse(mouseLoc);
		localToGlobal(mouseLoc);

		if not (ptInRgn(mouseLoc, mouseMoveRgn)) then
			begin
				countdown := 0;
				movergn.left := mouseLoc.h - 2;
				moveRgn.right := mouseLoc.h + 2;
				moveRgn.top := mouseLoc.v - 2;
				moveRgn.bottom := mouseLoc.v + 2;

				rectRgn(MouseMoveRgn, moveRgn);

			end;

{getKeys(theKeys);}
{for i := 0 to 127 do}
{if theKeys[i] then}
{countDown := 0;}

		if (countdown >= 180) then
			begin
				err := getFrontProcess(frontProcess);
				err := sameProcess(frontProcess, ourProcess, blah);
				if not blah then
					doScreenSave;
				countdown := 0;


			end;


	until doneFlag = true

end.