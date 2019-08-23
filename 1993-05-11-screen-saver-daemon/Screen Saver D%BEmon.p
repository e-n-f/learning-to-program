program Screen;

{$I-}

	var
		blah: integer;
		myPort: grafPtr;
		myEvent: eventRecord;
		mouseLoc: point;
		mouseMoveRgn: rgnHandle;
		MoveRgn: rect;
		test: boolean;
		countdown: longint;
		doneFlag: boolean;
		justDoneSave: integer;
		blah2: boolean;
		theKeys: keyMap;
		i: integer;

{ Insert your declarations here }

	procedure doScreenSave;

	begin
		blah := openDeskAcc(concat(chr(0), chr(0), 'Save Screen'));
	end;


begin
	initGraf(@thePort);

	flushEvents(everyEvent, 0);
	initcursor;

	myPort := GrafPtr(NewPtr(sizeOf(grafPort)));
	openport(myPort);
	MouseMoveRgn := newRgn;

	countdown := 0;
	setport(myPort);
	justDoneSave := 0;

	getMouse(mouseLoc);
	localToGlobal(mouseLoc);

	movergn.left := mouseLoc.h - 2;
	moveRgn.right := mouseLoc.h + 2;
	moveRgn.top := mouseLoc.v - 2;
	moveRgn.bottom := mouseLoc.v + 2;

	rectRgn(MouseMoveRgn, moveRgn);

	repeat

		getMouse(mouseLoc);
		localToGlobal(mouseLoc);

		movergn.left := mouseLoc.h - 2;
		moveRgn.right := mouseLoc.h + 2;
		moveRgn.top := mouseLoc.v - 2;
		moveRgn.bottom := mouseLoc.v + 2;

		rectRgn(MouseMoveRgn, moveRgn);

		if (getPixel(10, 0)) then
			begin
				countdown := 0;
				justdoneSave := 6
			end;

		if justDoneSave > 0 then
			justDoneSave := justDoneSave - 1;


		if (mouseLoc.h < 10) and (mouseLoc.v < 10) then
			countdown := maxInt;

		blah2 := waitNextEvent(everyEvent, myEvent, 60, nil);

{case MyEvent.what of}
{osEvt:}
{if band(brotl(myevent.message, 8), $ff) = mouseMovedMessage then}
{countdown := 0}
{end;}

		countdown := countdown + 1;

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

		getKeys(theKeys);
		for i := 0 to 127 do
			if theKeys[i] then
				countDown := 0;

		if (countdown >= 120) and (JustDoneSave = 0) then
			begin
				doScreenSave;
				countdown := 0;
				justDoneSave := 5;
			end

	until doneFlag = true

end.