program This;

	const
		appleID = 128;
		fileID = 129;

		menuCount = 2;

		appleM = 1;
		fileM = 2;



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
		growSize: longint;
		myPort: grafPtr;
		oldTime: str255;
		right: integer;

	procedure setUpMenus;

		var
			i: integer;

	begin
		myMenus[appleM] := getmenu(appleID);
		addResmenu(mymenus[appleM], 'DRVR');
		mymenus[fileM] := getmenu(fileID);

		for i := 1 to MenuCount do
			insertmenu(myMenus[i], 0);
		drawmenubar
	end;



	procedure updateClock;
		var
			i: integer;
			shortTime: str255;
			longTime: str255;
			timeInt: longInt;
			r: rect;
			theDate: str255;
			needDate: boolean;
			saving: boolean;
			theTime: str255;

	begin
		getTime(currentTime);
		getDateTime(timeInt);
		saving := false;
		setport(myPort);
		saving := getPixel(10, 0);
		if (currentTime.second <> prevTime.second) and not saving then
			begin
				oldTime := longTime;
				IUTimeString(timeInt, true, longTime);
				longTime := copy(longTime, 1, 8);
				if longTime[8] = ' ' then
					longTime := copy(longTime, 1, 7);
				iuDateString(timeInt, abbrevDate, theDate);
				theDate := copy(theDate, 6, 6);
				if theDate[6] = ',' then
					theDate := copy(theDate, 1, 5);
				textfont(21);
				textSize(9);
				longTime := concat('  ', longTime);
				moveto(right - stringwidth(longTime), 8);
				textmode(srcCopy);
				drawstring(longTime);
				theDate := concat('  ', theDate);
				moveto(right - stringwidth(theDate), 17);
				drawString(theDate);
				prevTime := currentTime;

			end;
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
						doneFlag := true
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
	right := myPort^.portRect.botRight.h - 36;


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

							end
						else
							begin
							end
					end;
				updateEvt: 
					begin
						beginUpdate(windowPtr(myEvent.message));

						endupdate(windowPtr(myEvent.message));

{updateClock(windowPtr(myEvent.message));}
					end
			end
		else

			updateclock;
	until doneFlag;
	disposePtr(ptr(myPort));
end.