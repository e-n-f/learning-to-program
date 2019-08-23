program This;

	const
		appleID = 128;
		fileID = 129;
		editID = 130;
		thisID = 131;
		fontID = 132;

		menuCount = 4;
		windowID = 128;
		otherWindowID = 129;

		appleM = 1;
		fileM = 2;
		editM = 3;
		thisM = 4;
		fontM = 5;

		maxActive = 16;

	type
		clockInfo = record
				analog: boolean;
				digital: boolean;
				date: (none, long, short, numerical);
{for clock #0, date = none for no screen saver detect, long for detect}
				active: boolean;
				wind: windowPtr;
				size: point;
				windowLoc: point
			end;

	var
		myMenus: array[1..menuCount] of MenuHandle;
		i: integer;
		dragrect: rect;
		doneflag: boolean;
		myWindow: windowptr;
		whichWindow: windowptr;
		myEvent: eventRecord;
		theChar: char;
		currenttime: dateTimeRec;
		prevTime: dateTimeRec;
		clocks: array[0..maxActive] of clockInfo;
		growSize: longint;
		ticks: array[1..12] of point;
		f: text;
		myPort: grafPtr;
		saveFile: file of clockInfo;
		oldTime: str255;
		analogCurs: cursHandle;
		digitalCurs: cursHandle;
	procedure EraseMBarClock;

	begin
		setPort(clocks[0].wind);
		penpat(white);
		paintrect(2 + clocks[0].windowLoc.h, 2 + clocks[0].windowLoc.v, 18 + clocks[0].windowLoc.h, 18 + clocks[0].windowLoc.v);
		moveto(1 + clocks[0].windowLoc.v, 3 + clocks[0].windowLoc.h);
		line(0, 14);
		moveto(2 + clocks[0].windowLoc.v, 2 + clocks[0].windowLoc.h);
		line(0, 13);
		moveto(3 + clocks[0].windowLoc.v, 1 + clocks[0].windowLoc.h);
		line(14, 0);
	end;

	procedure setUpMenus;

		var
			i: integer;

	begin
		myMenus[appleM] := getmenu(appleID);
		addResmenu(mymenus[appleM], 'DRVR');
		mymenus[fileM] := getmenu(fileID);
		mymenus[editM] := getmenu(editID);
		mymenus[thisM] := getmenu(thisID);

{mymenus[fontM] := getmenu(fontID);}
{addresmenu(mymenus[fontM], 'FONT');}

		for i := 1 to MenuCount do
			insertmenu(myMenus[i], 0);
		drawmenubar
	end;

	function seconds (time: dateTimeRec): real;

	begin
		seconds := time.second
	end;

	function hours (time: dateTimeRec): real;

	begin
		hours := (time.hour mod 12) + (time.minute / 60)
	end;

	function minutes (time: dateTimeRec): real;

	begin
		minutes := time.minute + (time.second / 60)
	end;

	function calc (arc: real; len: integer): point;
	begin
		calc.h := trunc(cos(2 * pi * arc + 3 * pi / 2) * len);
		calc.v := trunc(sin(2 * pi * arc + 3 * pi / 2) * len)
	end;

	procedure drawHand (arc, arc2: real; len: integer);
		var
			x1, x2, y1, y2: integer;
	begin
		moveto(50, 50);
		x1 := trunc(cos(2 * pi * arc + 3 * pi / 2) * len);
		y1 := trunc(sin(2 * pi * arc + 3 * pi / 2) * len);

		x2 := trunc(cos(2 * pi * arc2 + 3 * pi / 2) * len);
		y2 := trunc(sin(2 * pi * arc2 + 3 * pi / 2) * len);

		penpat(white);
		line(x1, y1);
		moveto(50, 50);
		penpat(black);
		line(x2, y2)
	end;

	procedure drawPartHand (whichhand: integer; win: windowPtr);

		var
			x1, y1, i: integer;

	begin
		i := getwrefcon(win);
		x1 := clocks[i].size.v;
		y1 := clocks[i].size.h;

		moveto(x1 div 2, y1 div 2);
		move(trunc(ticks[whichhand].h * 42 / 100 * x1 / 100), trunc(ticks[whichhand].v * 42 / 100 * y1 / 100));

		line(trunc(ticks[whichhand].h * 2 / 100 * x1 / 100), trunc(ticks[whichhand].v * 2 / 100 * y1 / 100))
	end;

{procedure drawHands (time: DateTimeRec);}

{begin}
{drawhand(hours(time) / 12, 25);}
{drawhand(minutes(time) / 60, 35);}
{drawhand(seconds(time) / 60, 40)}
{end;}



	procedure updateClock;
		var
			i: integer;
			h1, h2, m1, m2, s1, s2: point;
			x1, y1: integer;
			shortTime: str255;
			longTime: str255;
			timeInt: longInt;
			r: rect;
			theDate: str255;
			needDate: boolean;
			analogs: boolean;
			xh, yh: integer;
			saving: boolean;

	begin
		getTime(currentTime);
		getDateTime(timeInt);
		analogs := false;
		saving := false;
		if (clocks[0].date = long) and clocks[0].active then
			begin
				setport(clocks[0].wind);
				saving := getPixel(10, 0);
				setport(frontWindow)
			end;
		if currentTime.second <> prevTime.second then
			begin
				for i := 0 to maxActive do
					if clocks[i].analog and clocks[i].active then
						analogs := true;
				if analogs then
					begin
						h1 := calc(hours(prevTime) / 12, 25);
						h2 := calc(hours(currentTime) / 12, 25);
						m1 := calc(minutes(prevTime) / 60, 30);
						m2 := calc(minutes(currentTime) / 60, 30);
						s1 := calc(seconds(prevTime) / 60, 40);
						s2 := calc(seconds(currentTime) / 60, 40);
					end;
				oldTime := longTime;
				IUTimeString(timeInt, false, shortTime);
				IUTimeString(timeInt, true, longTime);
				if copy(shortTime, 2, 1) = ':' then
					shortTime := concat('0', shortTime);
				for i := 0 to maxActive do
					if clocks[i].active then
						begin
							needDate := false;
							x1 := clocks[i].size.v;
							y1 := clocks[i].size.h;
							setport(clocks[i].wind);
							case clocks[i].date of
								none: 
									begin
									end;
								short: 
									begin
										needDate := true;
										iuDateString(timeInt, abbrevDate, theDate)
									end;
								long: 
									begin
										needDate := true;
										iuDateString(timeInt, longDate, theDate)
									end;
								numerical: 
									begin
										needDate := true;
										iuDateString(timeInt, shortDate, theDate)
									end;
							end;

							xh := 0;
							yh := 0;

							if i = 0 then
								if clocks[i].analog then
									begin
										if ((currentTime.second) mod 5 = 0) and not saving then
											eraseMBarClock;
										penpat(black);
										frameoval(1 + clocks[0].windowLoc.h, 1 + clocks[0].windowLoc.v, 18 + clocks[0].windowLoc.h, 18 + clocks[0].windowLoc.v);
										xh := clocks[0].windowLoc.v;
										yh := clocks[0].windowLoc.h;
									end;

							if clocks[i].analog then
								begin
									if not saving then
										begin
											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(white);
											line(h1.h * x1 div 100, h1.v * y1 div 100);
											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(black);
											line(h2.h * x1 div 100, h2.v * y1 div 100);

											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(white);
											line(m1.h * x1 div 100, m1.v * y1 div 100);
											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(black);
											line(m2.h * x1 div 100, m2.v * y1 div 100);

											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(white);
											line(s1.h * x1 div 100, s1.v * y1 div 100);
											moveto(x1 div 2 + xh, y1 div 2 + yh);
											penpat(black);
											line(s2.h * x1 div 100, s2.v * y1 div 100);
										end
								end;
							textfont(21);
							if clocks[i].digital then
								begin
									setport(clocks[i].wind);
									if (i = 0) then
										begin
											if (currentTime.second mod 5 = 0) then
												begin
													if not saving then
														erasemBarClock;
												end;

											if not saving then
												begin
													textsize(9);
													textMode(srcCopy);
													setrect(r, clocks[0].windowLoc.v, clocks[0].windowLoc.h, clocks[0].windowLoc.v + 15, clocks[0].windowLoc.h + 18);
													cliprect(r);
													moveto(3 + clocks[0].windowLoc.v, 9 + clocks[0].windowLoc.h);
													if (currentTime.second) mod 2 = 0 then
														drawstring(copy(shortTime, 1, 3))
													else
														drawstring(concat(copy(shortTime, 1, 2), ' '));
													moveto(3 + clocks[0].windowLoc.v, 18 + clocks[0].windowLoc.h);
													drawstring(concat(copy(shortTime, 4, 2), ' '));
													cliprect(clocks[0].wind^.portRect);
												end
										end
									else
										begin
											if clocks[i].analog then
												begin
													setrect(r, 2, y1 + 2, x1, y1 + 15);
{windowPeek(clocks[i].wind)^.strucRgn^^.rgnBBox.right - 16, y1 + 14);}
													cliprect(r);
													textsize(12);
													textMode(srcCopy);
													moveto(2, y1 + 13);
													drawstring(longTime);
													if needDate then
														drawString(concat(' ', theDate));
													cliprect(clocks[i].wind^.portRect);
{setclip(windowPeek(clocks[i].wind)^.strucRgn)}
												end
											else
												begin
													textsize(y1);
													setrect(r, 2, 1, x1, y1);
													cliprect(r);
													textMode(srcCopy);
													moveto(2, y1 - (y1 div 8));
													drawstring(longTime);
													cliprect(clocks[i].wind^.portRect);
													if NeedDate and (i <> 0) then
														begin
															setrect(r, 2, y1 + 2, x1, y1 + 15);
															cliprect(r);
															textsize(12);
															moveto(2, y1 + 13);
															drawString(concat(theDate));
															cliprect(clocks[i].wind^.portRect);

														end;

												end

										end;
								end
							else if NeedDate and (i <> 0) then
								begin
									if clocks[i].analog then
										begin
											setrect(r, 2, y1 + 2, x1, y1 + 15);
											cliprect(r);
											textsize(12);
											moveto(2, y1 + 13);
											drawString(concat(theDate));
											cliprect(clocks[i].wind^.portRect);
										end
									else
										begin
											textsize(y1);
											setrect(r, 2, 1, x1, y1);
											cliprect(r);
											textMode(srcCopy);
											moveto(2, y1 - (y1 div 8));
											drawstring(theDate);
											cliprect(clocks[i].wind^.portRect);
										end

								end;
{penpat(white);}
{drawhands ( prevTime );}
{penpat ( black );}
{drawHands ( currentTime );}
						end;
				prevTime := currentTime;

			end;
	end;


	procedure Drawclock (window: windowPtr);

		var
			a: longint;
			i: integer;
			bla: integer;
			x, y: longint;

	begin
		setport(window);
		cliprect(window^.portRect);
		eraserect(window^.portRect);
		bla := getWrefcon(window);
		drawGrowIcon(window);
		if clocks[bla].analog then
			begin
				x := clocks[bla].size.h;
				y := clocks[bla].size.v;
				penpat(black);
				frameoval(trunc(x * 5 / 100), trunc(5 * y / 100), trunc(95 * (x + 1) / 100), trunc(95 * (y + 1) / 100));
				for i := 1 to 12 do
					drawparthand(i, window);
			end


	end;



	function NewClock: windowPtr;

		var
			i: integer;
			found: integer;
			ourWindow: windowPtr;
			lastOne: integer;

	begin
		found := 0;
		lastOne := 0;
		if frontWindow <> nil then
			lastOne := getWRefCon(frontWindow);
		for i := maxActive downto 1 do
			if clocks[i].active = false then
				found := i;
		if found = 0 then
			begin
				found := alert(131, nil);
				newClock := nil
			end
		else
			begin
				writeln('making clock ', found, ' from clock ', lastOne);
				ourWindow := getNewWindow(OtherwindowID, nil, pointer(-1));


				if lastOne = 0 then
					begin
						clocks[found].analog := true;
						clocks[found].digital := false;
						clocks[found].date := none;
						clocks[found].active := true;
						clocks[found].wind := ourWindow;
						clocks[found].size.h := 84;
						clocks[found].size.v := 84;
					end
				else
					begin
						clocks[found].analog := clocks[lastOne].analog;
						clocks[found].digital := clocks[lastOne].digital;
						clocks[found].date := clocks[lastOne].date;
						clocks[found].wind := ourWindow;
						clocks[found].size := clocks[lastOne].size;
						sizeWindow(ourWindow, clocks[found].size.v + 16, clocks[found].size.h + 16, true);
						clocks[found].active := true;
					end;

				setwrefcon(ourWindow, found);
				showWindow(ourWindow);
				newClock := ourWindow
			end


	end;

	procedure saveSettings;

		var
			i: integer;

	begin

		rewrite(saveFile, 'Clock Settings');
		for i := 0 to maxActive do
			begin

				if clocks[i].active and (i > 0) then
					begin
{clocks[i].windowLoc.h := windowPeek(clocks[i].wind)^.strucRgn^^.rgnBBox.left;}
{clocks[i].windowLoc.v := windowPeek(clocks[i].wind)^.strucRgn^^.rgnBBox.top;}
						clocks[i].windowLoc.h := clocks[i].wind^.portRect.left;
						clocks[i].windowLoc.v := clocks[i].wind^.portRect.top;
						setPort(clocks[i].wind);
						localToGlobal(clocks[i].windowLoc);
						writeln(i, clocks[i].windowLoc.h, clocks[i].windowLoc.v);
					end;
				write(saveFile, clocks[i])
			end;
		close(saveFile)


	end;

	procedure MakeWindow (i: integer);

		var

			ourWindow: windowPtr;

	begin
		begin
			writeln('making window', i);
			ourWindow := getNewWindow(OtherwindowID, nil, pointer(-1));
			clocks[i].wind := ourWindow;
			sizeWindow(ourWindow, clocks[i].size.v + 16, clocks[i].size.h + 16, true);
			setwrefcon(ourWindow, i);
			moveWindow(ourWindow, clocks[i].windowLoc.h, clocks[i].windowLoc.v, true);
			showWindow(ourWindow);
			writeln(i, clocks[i].windowLoc.h, clocks[i].windowLoc.v);
		end


	end;


	procedure KillWindow (whichWindow: windowPtr);

		var
			i: integer;

	begin
		i := getWRefCon(whichWindow);
		disposeWindow(whichWindow);
		clocks[i].active := false

	end;

	procedure FixMenu;

		var
			current: clockInfo;
			i: integer;
			thisM: menuHandle;


	begin

		thisM := getMHandle(thisID);

		if frontWindow = nil then
			begin
				disableItem(thisM, 5);
				disableItem(thisM, 6);
				disableItem(thisM, 8);
				disableItem(thisM, 9);
				disableItem(thisM, 10);
				disableItem(thisM, 11);
			end
		else
			begin
				enableItem(thisM, 5);
				enableItem(thisM, 6);
				enableItem(thisM, 8);
				enableItem(thisM, 9);
				enableItem(thisM, 10);
				enableItem(thisM, 11);
				current := clocks[getWrefCon(frontWindow)];
				checkItem(thisM, 5, current.analog);
				checkItem(thisM, 6, current.digital);
				for i := 8 to 11 do
					setItemMark(thisM, i, chr(noMark));
				checkItem(thisM, ord(current.date) + 8, true);

			end;
		for i := 1 to 3 do
			checkItem(thisM, i, false);
		if clocks[0].active then
			begin
				if clocks[0].analog then
					checkItem(thisM, 2, true);
				if clocks[0].digital then
					checkItem(thisM, 3, true)
			end
		else
			checkItem(thisM, 1, true);
		checkItem(thisM, 14, false);
		if clocks[0].date = long then
			checkItem(thisM, 14, true);



	end;




	procedure getBarLoc;

		var
			theCurs: cursHandle;
			theLoc: point;
			blah: boolean;
			event: eventRecord;

	begin
		if clocks[0].analog then
			setCursor(analogCurs^^)
		else
			SetCursor(digitalCurs^^);
		setPort(clocks[0].wind);
		repeat
			getMouse(theLoc)
		until Button;
		setCursor(arrow);
		blah := waitNextEvent(mDownMask, event, 0, nil);
		eraseMBarClock;
		clocks[0].windowLoc.h := theLoc.v - 1;
		clocks[0].windowLoc.v := theLoc.h - 1;
		eraseMBarClock;
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
		writeln('active clock', getwrefcon(frontWindow));
		if frontWindow <> nil then
			writeln(clocks[getWrefcon(frontwindow)].active);
		case theMenu of
			appleID: 
				begin
					getItem(myMenus[appleM], theItem, name);
					if theitem <> 1 then
						begin
							temp := openDeskAcc(name);
							setPort(myWindow)
						end
					else
						begin
							temp := alert(130, nil);
							if temp = 2 then
								temp := alert(132, nil);
						end

				end;

			fileID: 
				case theItem of
					1: 
						begin
							myWindow := newClock;
							fixMenu
						end;
					2: 
						begin
							killWindow(frontWindow);
							fixMenu
						end;
					3: 
						saveSettings;
					5: 
						doneFlag := true
				end;
			thisID: 
				case theItem of
					1:  {no menu bar}
						begin
							clocks[0].active := false;
							eraseMBarClock;
							fixMenu
						end;
					2:  {analog menu bar}
						begin
							clocks[0].active := true;
							clocks[0].analog := true;
							clocks[0].digital := false;
							eraseMBarClock;
							fixMenu
						end;
					3:  { digital menu bar }
						begin
							clocks[0].active := true;
							clocks[0].analog := false;
							clocks[0].digital := true;
							eraseMBarClock;
							fixMenu
						end;

					5:  {analog}
						begin
							clocks[GetWrefCon(frontWindow)].analog := not (clocks[GetWrefCon(frontWindow)].analog);
							fixMenu;
							drawClock(frontWindow);
						end;
					6:   {digital}
						begin
							clocks[GetWrefCon(frontWindow)].digital := not (clocks[GetWrefCon(frontWindow)].digital);
							fixMenu;
							drawClock(frontWindow);
						end;

					8: {no date}
						begin
							clocks[GetWrefCon(frontWindow)].date := none;
							fixMenu;
							drawClock(frontWindow);
						end;
					9: {full date}
						begin
							clocks[GetWrefCon(frontWindow)].date := long;
							fixMenu;
							drawClock(frontWindow);
						end;
					10: {short date}
						begin
							clocks[GetWrefCon(frontWindow)].date := short;
							fixMenu;
							drawClock(frontWindow);
						end;
					11: {num. date}
						begin
							clocks[GetWrefCon(frontWindow)].date := numerical;
							fixMenu;
							drawClock(frontWindow);
						end;
					13: {set mbar clock loc}
						getBarLoc;
					14: {detect screen saver}
						begin
							if clocks[0].date = none then
								clocks[0].date := long
							else
								clocks[0].date := none;
							fixMenu;
						end;
				end;
		end;



		Hilitemenu(0);
	end;


begin
	flushEvents(everyEvent, 0);
	initcursor;
	getTime(prevTime);

	SetupMenus;
	with ScreenBits.bounds do
		setrect(dragrect, 4, 24, right - 4, bottom - 4);
	doneflag := false;
{myWindow := newClock;}
	myPort := GrafPtr(NewPtr(sizeOf(grafPort)));
	openport(myPort);

{setport(myWindow);}


	clocks[0].analog := false;
	clocks[0].digital := false;
	clocks[0].date := none;
	clocks[0].active := false;
	clocks[0].size.h := 18;
	clocks[0].size.v := 18;
	clocks[0].windowLoc.h := 0;
	clocks[0].windowLoc.v := 0;

	open(saveFile, 'Clock settings');
	close(saveFile);
	reset(saveFile, 'Clock settings');
	if not eof(saveFile) then
		for i := 0 to maxActive do
			begin
				read(saveFile, clocks[i]);
				writeln(i, clocks[i].active);
				if clocks[i].active and (i > 0) then
					makeWindow(i)
			end;
	close(saveFile);
	clocks[0].wind := myPort;

	for i := 1 to 12 do
		begin
			ticks[i].h := trunc(100 * cos(i / 12 * pi * 2));
			ticks[i].v := trunc(100 * sin(i / 12 * pi * 2))
		end;

	fixMenu;
{for i := 0 to 120 do}
{begin}
{sintable[i] := sin(i * 2 * pi / 120);}
{sintable[i + 120] := sintable[i];}
{costable[i] := cos(i * 2 * pi / 120);}
{costable[i + 120] := costable[i];}
{drawparthand((i - 90) / 120, 10, 20)}
{end;}

	analogCurs := getCursor(128);
	digitalCurs := getCursor(129);

	if clocks[0].active then
		EraseMBarClock;

	repeat
		oldTime := '';
		if waitnextevent(everyevent, myEvent, 20, nil) then
			case myEvent.what of
				mouseDown: 
					case FindWindow(myEvent.where, whichWindow) of
						inSysWindow: 
							SystemClick(myEvent, whichWindow);
						inMenuBar: 
							doCommand(menuSelect(myEvent.where));
						inDrag: 
							begin
								dragWindow(whichWindow, myEvent.where, dragRect);
{drawClock}
							end;
						inContent: 
							begin
								if whichWindow <> frontWindow then
									begin
										selectWindow(whichWindow);
										fixMenu
									end
								else
									begin
									end;
							end;
						inGrow: 
							begin
								growsize := GrowWindow(whichWindow, myEvent.where, dragRect);
								if growsize <> 0 then
									begin
										sizeWindow(whichWindow, loWord(growSize), hiWord(growSize), true);
										i := getWrefcon(whichWindow);
										clocks[i].size.h := hiWord(growSize) - 16;
										clocks[i].size.v := loWord(growSize) - 16;
										drawClock(whichWindow)
									end
							end;

						inGoAway: 
							if trackGoAway(whichWindow, myEvent.where) then
								begin
									killWindow(whichWindow);
									fixMenu
								end
					end;
				keydown, autokey: 
					begin
						theChar := chr(bitAnd(MyEvent.message, charCodeMask));
						if bitand(myEvent.modifiers, cmdKey) <> 0 then
							doCommand(menuKey(theChar))
					end;
				activateEvt: 
					begin
						if BitAnd(myEvent.modifiers, activeFlag) <> 0 then
							begin
{drawClock}
								fixMenu
							end
						else
							begin
							end
					end;
				updateEvt: 
					begin
						beginUpdate(windowPtr(myEvent.message));
						drawclock(windowPtr(myEvent.message));
						endupdate(windowPtr(myEvent.message));

{updateClock(windowPtr(myEvent.message));}
					end
			end
		else

			updateclock;
	until doneFlag;
	for i := 1 to maxActive do
		if clocks[i].active then
			killWindow(clocks[i].wind);
	disposePtr(ptr(myPort));
end.