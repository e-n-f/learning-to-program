program This;

{$I-}

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
		whichWindow: windowptr;
		myEvent: eventRecord;
		currenttime: dateTimeRec;
		prevTime: dateTimeRec;
		myPort: grafPtr;
		right: integer;
		temp: boolean;

		longTime: str255;
		timeInt: longInt;
		r: rect;
		theDate: str255;
		saving: boolean;
		theTime: str255;


	procedure myDrawString (s: str255);

		var
			i: integer;

	begin
		for i := 1 to length(s) do
			drawChar(s[i]);
	end;




begin
	initGraf(@thePort);
	initFonts;

	flushEvents(everyEvent, 0);
	initcursor;
	getTime(prevTime);

	doneflag := false;
{myWindow := newClock;}
	myPort := GrafPtr(NewPtr(sizeOf(grafPort)));
	openport(myPort);
	right := myPort^.portRect.botRight.h - 36;
	setport(myport);
	textfont(21);
	textSize(9);

	repeat
		temp := waitnextevent(everyevent, myEvent, 20, nil);
		begin
			getTime(currentTime);
			getDateTime(timeInt);
			saving := false;
			setport(myPort);
			saving := getPixel(10, 0);
			if (currentTime.second <> prevTime.second) and not saving then
				begin
					moveto(60, 60);
					myDrawString('test');


					penpat(white);
					moveto(CurrentTime.minute, currentTime.second);
					lineto(currentTime.hour, currentTime.minute);
					IUTimeString(timeInt, true, longTime);
					longTime := copy(longTime, 1, 8);
					if longTime[8] = ' ' then
						longTime := copy(longTime, 1, 7);
					iuDateString(timeInt, abbrevDate, theDate);
					theDate := copy(theDate, 6, 6);
					if theDate[6] = ',' then
						theDate := copy(theDate, 1, 5);
					longTime := concat('  ', longTime, ' <-');
					moveto(40, 40);
{(right - stringwidth(longTime), 8);}
					textmode(srcCopy);
					mydrawstring(longTime);
					theDate := concat('  ', theDate, ' <-');
					moveto(40, 48);

{moveto(right - stringwidth(theDate), 17);}
					mydrawString(theDate);
					prevTime := currentTime;
				end;
		end
	until doneFlag;
	disposePtr(ptr(myPort));
end.