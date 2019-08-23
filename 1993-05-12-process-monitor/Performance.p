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
		maxSimProcs = 50;
		tickWidth = 1;

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
		setOfProcTimes = array[1..maxSimProcs] of longInt;
		setOfProcNums = array[1..maxSimProcs] of processSerialNumber;
		sampleRec = record
				length: integer; {length in ticks of sample}
				timesUsed: setOfProcTimes;
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

		prevTime, thisTime: setOfProcTimes;

		lastTime, curTime: longInt;
		onesUsed: integer; {local process numbers assigned}
		theGlobProcNums: setofProcNums;

		lastTimeChange: SetOfProcTimes;


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

	procedure processInfo;

		var
			i: integer;
			localProcNum: integer;


		function findLocalProcNum (realNum: ProcessSerialNumber): integer;
			var
				i: integer;
				foundIt: boolean;

		begin
			foundIt := false;
			for i := 1 to onesUsed do
				begin
					if theGlobProcNums[i].lowLongOfPSN = realNum.lowLongOfPSN then
						begin
							foundIt := true;
							findLocalProcNum := i;
						end;
				end;
			if foundIt = false then
				begin
					onesUsed := onesUsed + 1;
					theGlobProcNums[onesUsed] := realNum;
					findLocalProcNum := onesUsed;
				end;

		end;

	begin
		prevTime := thisTime;
		for i := 1 to maxSimProcs do
			thisTime[i] := 0;
		for i := 1 to numProcesses do
			begin
				localProcNum := findLocalProcNum(processNums[i]);
				thisTime[localProcNum] := apps[i].activeTime;
			end;
	end;


	procedure setColors (i: integer);
	begin
		foreColor(theColors[i mod 6]);
		backColor(theColors[i mod 6]);
		penpat(gray);
	end;




	procedure paintInfo;
		var
			i, j, k: integer;

			sa: str255;

			rect1, rect2, rect3: rect;
			timeDif: longInt;

			theWidth, theDepth: integer;

			timeChange: SetOfProcTimes;
			curDepth, yaCurDepth: integer;

			myPoly: polyHandle;

	begin
		setport(myWindow);
		cliprect(myWindow^.portRect);
		textMode(srcCopy);
		curTime := tickCount;
		timeDif := curTime - lastTime;

		theWidth := myWindow^.portRect.botRight.h - timeDif * tickWidth;
		theDepth := myWindow^.portRect.botRight.v;

		with rect1 do
			begin
				topLeft.h := timeDif * tickWidth;
				topLeft.v := 0;

				botRight.h := theWidth + timeDif * tickWidth;
				botRight.v := theDepth;
			end;

		with rect2 do
			begin
				topLeft.h := 0;
				topleft.v := 0;
				botRight.h := theWidth;
				botRight.v := theDepth;
			end;

		with rect3 do
			begin
				topLeft.h := theWidth;
				topLeft.v := 0;

				botRight.h := theWidth + timeDif * tickWidth;
				botRight.v := theDepth;
			end;

		CopyBits(myWindow^.portBits, myWindow^.portBits, rect1, rect2, srcCopy, nil);
		eraseRect(rect3);

		curDepth := theDepth;
		yaCurDepth := theDepth;
		begin
			for i := 1 to onesUsed do
				begin
					timeChange[i] := thisTime[i] - prevTime[i];
					if timeChange[i] < 0 then
						timeChange[i] := 0;
					prevTime[i] := thisTime[i];


					myPoly := openPoly;
					moveto(theWidth + timeDif * tickWidth, curDepth - (theDepth * timeChange[i] div timeDif));
					lineto(theWidth, yaCurDepth - lastTimeChange[i]);
					lineto(theWidth, yaCurDepth);
					lineto(theWidth + timeDif * tickWidth, curDepth);
					closePoly;

					setColors(i);
					FillPoly(myPoly, gray);
					killPoly(myPoly);

					foreColor(blackColor);
					backColor(whiteColor);
					penPat(black);

					moveto(theWidth + timeDif * tickWidth, curDepth - (theDepth * timeChange[i] div timeDif));
					lineto(theWidth, yaCurDepth - lastTimeChange[i]);


					yaCurDepth := yaCurDepth - lastTimeChange[i];
					lastTimeChange[i] := theDepth * timeChange[i] div timeDif;
					curDepth := curDepth - lastTimeChange[i];
				end;
		end;
		lastTime := curTime;


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
		processInfo;
		paintInfo;
		cliprect(myWindow^.portRect);


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
	lastTime := TickCount;

	SetupMenus;
	with ScreenBits.bounds do
		setrect(dragrect, 4, 24, right - 4, bottom - 4);
	doneflag := false;
	myWindow := getNewWindow(128, nil, pointer(-1));
	myStringPtr := stringPtr(NewPtr(sizeOf(thisString)));


	needAFix := true;
	needRefresh := false;
	firstTime := true;


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

		if false then
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