program What;

	uses
		Processes;

	const
		menuCount = 6;

		appleM = 1;
		fileM = 2;
		editM = 3;
		viewM = 4;
		memM = 5;
		timeM = 6;

		appleID = 128;
		fileID = 129;
		editID = 130;
		viewID = 131;
		memID = 132;
		timeID = 132;

		mpName = 1;
		mcpuUse = 2;
		mpSig = 3;
		mtStarted = 4;
		meTime = 6;
		meTicks = 7;
		muTime = 9;
		muTicks = 10;

		maxNum = 32;

	type
		thisString = string[255];
		ProcessStuff = record
				name: string;
				signature: osType;
				size: longInt;
				freeMem: longInt;
				launchDate: longInt;
				activeTime: longInt;
				address: longInt;
			end;
		memRec = record
				localNum: integer;
				address: longInt;
			end;


{ Insert your declarations here }

	var
		myMenus: array[1..menuCount] of menuHandle;
		dragrect: rect;
		doneflag: boolean;
		myWindow, memWindow, loadWindow: windowptr;
		whichWindow: windowptr;
		myEvent: eventRecord;
		theChar: char;
		growSize: longInt;
		myStringPtr: stringPtr;
		osReturn: osErr;
		apps: array[1..maxNum] of processStuff;
		prevApps: array[1..maxNum] of processStuff;
		memOrder: array[1..maxNum] of memRec;

		pName: boolean;
		cpuUse: boolean;
		pSig: boolean;
		tStarted: boolean;
		eTime: boolean;
		eTicks: boolean;
		uTime: boolean;
		uTicks: boolean;

		numProcesses, oldNumProcs: integer;
		processNums: array[1..maxNum] of processSerialNumber;
		needAFix: boolean; { 'cos I'm going down }
		needRefresh: boolean;
		firstTime: boolean; {there's a first time for everything}

		lastFix: longInt;
		nameWid: longInt;

		ramSize, loMem: longInt;

		r: rect;

		theColors: array[0..5] of integer;

		updateName: boolean;
		updateSize: boolean;
		updateAll: boolean;

	procedure setUpMenus;

		var
			i: integer;

	begin
		myMenus[appleM] := getmenu(appleID);
		addResmenu(mymenus[appleM], 'DRVR');
		mymenus[fileM] := getmenu(fileID);
		mymenus[editM] := getmenu(editID);
		mymenus[viewM] := getmenu(viewID);
		mymenus[memM] := getmenu(memID);
		myMenus[timeM] := getMenu(timeID);

		for i := 1 to MenuCount do
			insertmenu(myMenus[i], 0);
		drawmenubar
	end;


	procedure CheckIt (thisID: integer; mItem: integer; var Item: boolean);

		var
			blueM: menuHandle;
			ya: char;

	begin
		blueM := getMHandle(thisID + 127);
		Item := false;
		getItemMark(blueM, mItem, ya);
		if ord(ya) <> 0 then
			Item := true;
	end;




	procedure drawStuff (window: windowPtr);
		var
			r: rect;
	begin
		setport(window);
		cliprect(window^.portRect);
		eraserect(window^.portRect);
		if window = memWindow then
			begin
				with window^.portRect.botRight do
					setRect(r, h - 15, v - 15, h, v);
				clipRect(r);
				drawGrowIcon(window);
				clipRect(window^.portRect);
			end;
		needRefresh := true;
{drawGrowIcon(window);}
	end;

	procedure FixMenu;

		var
			viewM: menuHandle;

	begin
		viewM := getMHandle(viewID);

		checkItem(viewM, mpName, pName);
		checkItem(viewM, mcpuUse, cpuUse);
		checkItem(viewM, mpSig, pSig);
		checkItem(viewM, mtStarted, tStarted);
		checkItem(viewM, meTime, eTime);
		checkItem(viewM, meTicks, eTicks);
		checkItem(viewM, muTicks, uTicks);
		checkItem(viewM, muTime, uTime);

	end;

	procedure getInfo;

		var

			myProcessInfoRec: processInfoRec;
			thisProcess: processSerialNumber;
			i: integer;

	begin

		numProcesses := 1;
		thisProcess.highlongofPsn := 0;
		thisProcess.lowLongOfPSN := kNoProcess;
		repeat
			osReturn := getNextProcess(thisProcess);
			processNums[numProcesses] := thisProcess;
			numProcesses := numProcesses + 1;
		until thisProcess.lowLongofPSN = kNoProcess;
		numProcesses := numProcesses - 2;


		for i := 1 to numProcesses do
			begin
				myProcessInfoRec.processInfoLength := sizeOf(myProcessInfoRec);
				myProcessInfoRec.processName := myStringPtr;
				myProcessInfoRec.processAppSpec := nil;
				osReturn := getProcessInformation(processNums[i], myProcessInfoRec);
				apps[i].name := myProcessInfoRec.processName^;
				apps[i].signature := myProcessInfoRec.processSignature;
				apps[i].size := myProcessInfoRec.processSize;
				apps[i].freeMem := myProcessInfoRec.processFreeMem;
				apps[i].launchDate := myProcessInfoRec.processLaunchDate;
				apps[i].activeTime := myProcessInfoRec.processActiveTime;
				apps[i].address := longInt(myProcessInfoRec.processLocation);

				if (apps[i].freeMem) div 1024 <> (prevApps[i].freeMem) div 1024 then
					updateSize := true;
				if apps[i].name <> prevApps[i].name then
					updateName := true;

				prevApps[i] := apps[i];
			end;

	end;

	function cpu (i: integer): string;
		var
			blah: str255;
			a: integer;
			foo: longint;
	begin
		if tickCount - apps[i].launchDate = 0 then
			cpu := '100.0'
		else
			begin
				foo := 1000 * (apps[i].activeTime);
				foo := foo div (tickCount - apps[i].launchDate);
				numToString(foo, blah);
				a := length(blah);
				blah := concat(copy(blah, 1, a - 1), '.', copy(blah, a, 1));
				cpu := blah;
			end
	end;

	function sig (i: integer): string;
		var
			s: string;
			k: integer;
	begin
		s := '';
		for k := 1 to 4 do
			s := concat(s, apps[i].Signature[k]);
		sig := s;
	end;

	function pad2 (t: longInt): string;
		var
			s: str255;
	begin
		numToString(t, s);
		pad2 := s;
	end;

	function pad (t: longInt): string;

		var
			s: str255;

	begin
		numtoString(t, s);
		if length(s) = 1 then
			s := concat('0', s);
		pad := s;
	end;

	function tickToTime (t: longInt): string;

	begin
		t := t div 60;
		tickToTime := concat(pad2(t div 3600), ':', pad((t div 60) mod 60), ':', pad(t mod 60));
	end;

	function tstart (i: integer): string;
	begin
		tstart := TickToTime(apps[i].launchDate);
	end;

	function telapse (i: integer): string;
		var
			blah: string;
	begin
		telapse := tickToTime(tickCount - apps[i].launchDate);

	end;

	function tickElapse (i: integer): string;
		var
			s: str255;
	begin
		numToString(TickCount - apps[i].launchDate, s);
		tickElapse := s;
	end;

	function tUsed (i: integer): string;
	begin
		tUsed := tickToTime(apps[i].activeTime);
	end;

	function tickUsed (i: integer): string;
		var
			s: str255;
	begin
		numToString(apps[i].activeTime, s);
		tickUsed := s;
	end;


	procedure drawIt (i, w: integer; s: string);

	begin
		textsize(9);
		moveto(w, 12 * i + 10);
		drawString(s);
	end;

	procedure drawGraph (num, stCol, endCol: longInt);

		var
			totalTicks, ourUsedTicks, ourTotalTicks: longInt;
			theWidth: longInt;
			otherEndCol: longInt;
			thisThing: integer;
			thisUsed, thisTotal: longInt;

	begin
		penpat(black);
		otherEndCol := endCol;
		endCol := endCol - 20;
		totalTicks := tickCount;
		ourUsedTicks := apps[num].activeTime;
		ourTotalTicks := TickCount - apps[num].launchDate;
		theWidth := endCol - stCol;

		thisUsed := trunc(endCol - (theWidth * ourUsedTicks / totalTicks));
		thisTotal := trunc(endCol - (theWidth * ourTotalTicks / totalTicks));

{eraserect(12 * num + 3, thisTotal + 1, 12 * num + 11, endCol - 1);}

		penpat(white);
		moveto(thisTotal - 1, 12 * num + 3);
		line(0, 8);
		moveto(thisTotal - 2, 12 * num + 3);
		line(0, 8);
		moveto(thisTotal + 1, 12 * num + 3);
		line(0, 8);
		moveto(thisTotal + 2, 12 * num + 3);
		line(0, 8);
		moveto(thisUsed - 1, 12 * num + 3);
		line(0, 8);
		moveto(thisUsed - 2, 12 * num + 3);
		line(0, 8);
		penpat(black);
		framerect(12 * num + 2, thisTotal, 12 * num + 12, endCol);
{eraseRect(12 * num + 2, trunc(endCol - (theWidth * ourTotalTicks / totalTicks)), 12 * num + 12, endCol);}

		foreColor(redColor);
		paintrect(12 * num + 3, thisUsed, 12 * num + 11, endCol - 1);
		foreColor(blackColor);

		stCol := endCol + 2;
		endCol := otherEndCol;
		theWidth := endCol - stCol;
{    eraseRect(12 * num + 2, stCol, 12 * num + 12, endCol);}


		thisTotal := trunc(endCol - (theWidth * ourUsedTicks / ourTotalTicks));
{eraseRect(12 * num + 3, stCol + 1, 12 * num + 11, endCol - 1);}

		penpat(white);
		moveto(thisTotal - 1, 12 * num + 3);
		line(0, 8);
		moveto(thisTotal - 2, 12 * num + 3);
		line(0, 8);
		penpat(black);

		framerect(12 * num + 2, stCol, 12 * num + 12, endCol);
		foreColor(redColor);
		paintrect(12 * num + 3, thisTotal, 12 * num + 11, endCol - 1);
		foreColor(blackColor);
	end;


	procedure paintInfo;
		var
			i, j, k: integer;

			wpName: integer;
			wcpuUse: integer;
			wpSig: integer;
			wtStarted: integer;
			weTime: integer;
			weTicks: integer;
			wuTime: integer;
			wuTicks: integer;
			bigWid: integer;
			cornBall: integer;

			localDone: boolean;

	begin
		setport(myWindow);
		cliprect(myWindow^.portRect);
		textfont(1);
		textsize(9);
		wpName := 2;
		if pName then
			begin
				wcpuUse := wpName + 5;
				for i := 1 to numProcesses do
					begin
						cornBall := stringWidth(apps[i].name);

						if cornBall > wcpuUse then
							wcpuUse := cornBall + 14;
					end;
			end
		else
			wcpuUse := wpName;
		if wcpuUse <> wpName then
			nameWid := wcpuUse;
		if cpuUse then
			wpSig := wcpuUse + 40
		else
			wpSig := wcpuUse;
		if pSig then
			wtStarted := wpSig + 50
		else
			wtStarted := wpSig;
		if tStarted then
			weTime := wtStarted + 90
		else
			weTime := wtStarted;
		if eTime then
			weTicks := weTime + 65
		else
			weTicks := weTime;
		if eTicks then
			wuTime := weTicks + 65
		else
			wuTime := weTicks;
		if uTime then
			wuTicks := wuTime + 55
		else
			wuTicks := wuTime;
		if uTicks then
			bigWid := wuTicks + 65
		else
			bigWid := wuTicks;

		if NeedAFix then
			begin
				setPort(memWindow);
				cliprect(memWindow^.portRect);
				eraseRect(memWindow^.portRect);
				drawStuff(memWindow);

				setPort(myWindow);
				cliprect(myWindow^.portRect);
				sizeWindow(myWindow, bigWid, numProcesses * 12 + 14, true);
				cliprect(myWindow^.portRect);
				needAFix := false;
				drawStuff(myWIndow);

				updateAll := true;
				needRefresh := true;
				memOrder[1].localNum := 0;
				memOrder[1].address := maxLongInt;
				for i := 1 to numProcesses do
					begin
						localDone := false;
						for j := 1 to numProcesses + 1 do
							if localDone = false then
								begin
									if apps[i].address < memOrder[j].address then
										begin
											for k := numProcesses downto j do
												memOrder[k + 1] := memOrder[k];
											memOrder[j].address := apps[i].address;
											memOrder[j].localNum := i;
											localDone := true;
										end
								end;
					end;

			end;

		textMode(srcCopy);
		textface([underline]);
		if updateAll then
			begin
				if pName then
					drawIt(0, wpName, 'Name');
				if cpuUse then
					drawIt(0, wcpuUse, '% CPU');
				if pSig then
					drawIt(0, wpSig, 'Signature');
				if tStarted then
					drawIt(0, wtStarted, 'CPU usage graph');
				if eTime then
					drawIt(0, weTime, 'Elapsed time');
				if eTicks then
					drawIt(0, weTicks, 'Elapsed ticks');
				if uTime then
					drawIt(0, wuTime, 'Time used');
				if uTicks then
					drawIt(0, wuTicks, 'Ticks used');
			end;

		textface([]);
		for i := 1 to numProcesses do
			begin
				if updateAll or updateName then
					if pName then
						drawIt(i, wpName, apps[i].name);
				if cpuUse then
					drawit(i, wcpuUse, cpu(i));
				if pSig then
					drawIt(i, wpSig, sig(i));
				if tStarted then
					drawGraph(i, wtStarted, weTime - 5);
{drawIt(i, wtStarted, tstart(i));}
				if eTime then
					drawIt(i, weTime, telapse(i));
				if eTicks then
					drawIt(i, weTicks, tickElapse(i));
				if uTime then
					drawIt(i, wuTime, tused(i));
				if uTicks then
					drawIt(i, wuTicks, tickUsed(i));
			end;
	end;

	procedure setColors (i: integer);
	begin
		foreColor(theColors[i mod 6]);
		backColor(theColors[i mod 6]);
		penpat(gray);
	end;

	procedure paintMemInfo;

		var
			bot, right, top, left: longint;
			mult: real;
			i, j, k: integer;
			localTop, localBot, breakPoint, wid: longInt;
			s: str255;
			localDone: boolean;

			myProcessInfoRec: processInfoRec;
			thisProcess: processSerialNumber;

			address, size, freeMem: longInt;

	begin
		left := 16;
		bot := memWindow^.portRect.botright.v - 16;
		right := left + 32;
		if memWindow^.portRect.botRight.h > 96 + 3 * numProcesses + nameWid then
			right := memWindow^.portRect.botRight.h - nameWid - 32 - 3 * numProcesses;

{right := memWindow^.portRect.botRight.h div 2;}
		top := 16;
		penpat(black);
		frameRect(left, top, bot, right);
		mult := (bot - top) / ramSize;

		wid := right - left;

		if updateAll or updateSize then {updateAll or}
			begin

				for i := 1 to numProcesses do
					begin



						localTop := trunc((apps[i].address) * mult) + top;
						localBot := trunc((apps[i].address + apps[i].size) * mult) + top;

						breakpoint := trunc(right - ((apps[i].freeMem / apps[i].size) * wid));

						setColors(i);

						paintRect(localTop + 1, left + 1, localBot, breakPoint);

						backColor(whiteColor);

						penpat(white);
						paintRect(localTop + 1, breakPoint + 1, localBot, right - 1);

						foreColor(blackColor);
						penPat(black);

						moveto(breakPoint, localTop + 1);
						line(0, localBot - localTop - 1);

{foreColor(blueColor);}
{penpat(ltGray);}
{paintRect(localTop+1, breakPoint + 1, localBot, right - 1);}


						penpat(black);
						moveto(left, localTop);
						line(right - left - 1, 0);

						moveto(left, localBot);
						line(right - left - 1, 0);


					end;

			end;

		for j := 1 to numProcesses do
			begin
				foreColor(blackColor);
				penpat(black);
				if updateName or updateAll then {updateAll or}
					begin
						moveto(right + 2, trunc((apps[memOrder[j].localNum].address + (apps[memOrder[j].localNum].size) div 2) * mult) + top);
						line(j * 3 + 2, 0);
						lineto(right + 2 + j * 3 + 2, top + 10 + j * 25 - 25);
						lineto(right + numProcesses * 3 + 8, top + 10 + j * 25 - 25);
						setColors(memOrder[j].localNum);
						paintRect(top + 10 + j * 25 - 25 - 9, right + numProcesses * 3 + 8 + 2, top + 10 + j * 25 - 5 - 9, right + numProcesses * 3 + 8 + 12);
						foreColor(blackColor);
						backColor(whiteColor);
						penpat(black);
						frameRect(top + 10 + j * 25 - 25 - 9 - 1, right + numProcesses * 3 + 8 + 2 - 1, top + 10 + j * 25 - 5 - 9 + 1, right + numProcesses * 3 + 8 + 12 + 1);
					end;
				moveto(right + numProcesses * 3 + 8 + 14 + 2, top + 10 + j * 25 - 25 - 3);

				textfont(geneva);


				textSize(9);
				textMode(srcCopy);
				if updateName or updateAll then {or updateAll}
					drawString(apps[memOrder[j].localNum].name);
				if updateSize or updateAll then {or updateAll}
					begin
						moveto(right + numProcesses * 3 + 8 + 14 + 2, top + 10 + j * 25 - 15 - 1);


						drawstring('(');
						numToString((apps[memOrder[j].localNum].size - apps[memOrder[j].localNum].freeMem) div 1024, s);
						drawstring(s);
						drawstring('K / ');
						numToString((apps[memOrder[j].localNum].size) div 1024, s);
						drawstring(s);
						drawString('K)');
					end
			end;
		if updateSize or updateAll then
			begin
				size := MFFreeMem;
				freeMem := MFMaxMem(size) div 1024;
				size := MFFreeMem div 1024;
				address := longInt(SystemZone);

				moveto(right + numProcesses * 3 + 8 + 2, top + 10 + (numProcesses + 2) * 25 - 25 - 3);
				drawString('Total free memory:');
				moveto(right + numProcesses * 3 + 8 + 2, top + 10 + (numProcesses + 2) * 25 - 15 - 1);
				NumToString(size, s);
				drawstring(s);
				drawString('K');

				moveto(right + numProcesses * 3 + 8 + 2, top + 10 + (numProcesses + 3) * 25 - 25 - 3);
				drawString('Largest block:');
				moveto(right + numProcesses * 3 + 8 + 2, top + 10 + (numProcesses + 3) * 25 - 15 - 1);
				NumToString(freeMem, s);
				drawstring(s);
				drawString('K');



			end;

	end;

	procedure updateStuff;
		var
			r: rect;
	begin
		setPort(myWindow);
		r := myWindow^.portRect;
		setrect(r, 0, 0, r.botRight.h - 16, r.botRight.v - 16);
{cliprect(r);}

		getInfo;
		paintInfo;
		cliprect(myWindow^.portRect);

		setPort(memWindow);
		paintMemInfo;
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
							temp := alert(128, nil);

						end

				end;

			fileID: 
				case theItem of
					1: 
						doneFlag := true
				end;
			viewID: 
				begin
					needAFix := true;
					needRefresh := true;

					case theItem of
						mpName: 
							pName := not pName;
						mcpuUse: 
							cpuUse := not cpuUse;
						mpSig: 
							pSig := not pSig;
						mtStarted: 
							tStarted := not tStarted;
						meTicks: 
							eTicks := not eTicks;
						meTime: 
							eTime := not eTime;
						muTime: 
							uTime := not uTime;
						muTicks: 
							uTicks := not uTicks;
					end;
					fixMenu;
				end;
		end;
		hiliteMenu(0);
	end;


begin
	flushEvents(everyEvent, 0);
	initcursor;

	SetupMenus;
	with ScreenBits.bounds do
		setrect(dragrect, 4, 24, right - 4, bottom - 4);
	doneflag := false;
	myWindow := getNewWindow(128, nil, pointer(-1));
	memWindow := getNewWindow(129, nil, pointer(-1));
	myStringPtr := stringPtr(NewPtr(sizeOf(thisString)));

	checkIt(viewM, mpName, pName);
	checkIt(viewM, mcpuUse, cpuUse);
	checkIt(viewM, mpSig, pSig);
	checkIt(viewM, mtStarted, tStarted);
	checkIt(viewM, meTime, eTime);
	checkIt(viewM, meTicks, eTicks);
	checkIt(viewM, muTicks, uTicks);
	checkIt(viewM, muTime, uTime);

	needAFix := true;
	needRefresh := false;
	firstTime := true;

	osReturn := gestalt(gestaltLogicalRamSize, ramSize);
	osReturn := gestalt(gestaltLowMemorySize, loMem);

	theColors[0] := redColor;
	theColors[1] := greenColor;
	theColors[2] := blueColor;
	theColors[3] := cyanColor;
	theColors[4] := magentaColor;
	theColors[5] := yellowColor;

	repeat
		if waitnextevent(everyevent, myEvent, 60, nil) then
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
										drawStuff(whichWindow);
										needRefresh := true;
										updateAll := true;
									end
							end;

						inGoAway: 
							if trackGoAway(whichWindow, myEvent.where) then
								begin
									doneFlag := true;
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
							end
						else
							begin
							end;

					end;
				updateEvt: 
					begin
						updateAll := true;
						beginUpdate(windowPtr(myEvent.message));
{drawStuff(windowPtr(myEvent.message));}
						if windowPtr(myEvent.message) = memWindow then
							begin
								setPort(memWindow);
								with memwindow^.portRect.botRight do
									setRect(r, h - 15, v - 15, h, v);
								clipRect(r);
								drawGrowIcon(memwindow);
								clipRect(memwindow^.portRect);
							end;
						endupdate(windowPtr(myEvent.message));
						needRefresh := true;
{updateClock(windowPtr(myEvent.message));}
					end
			end
		else
			begin
				updateStuff;
				needRefresh := false;
			end;
		if needRefresh then
			begin
				updateStuff;
				needRefresh := false;
			end;

		if numProcesses <> oldnumProcs then
			begin
				if firstTime then
					begin
						firstTime := false;

					end
				else
					begin
						drawStuff(myWindow);
						needAFix := true;
					end
			end;
		oldNumProcs := numProcesses;
		updateAll := false;
		updateSize := false;
		updateName := false;
	until DoneFlag;

end.