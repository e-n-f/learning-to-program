100	call rech ::
	dim wrd$(30)
110	!
120	call char(143, "FFFFFFFFFFFFFFFF")

130	! Title screen
140	call clear
150	for a = 1 to 24 ::
		print ::
	next a

160	call magnify(2) ::
	gosub 350 ::
	gosub 440 ::
	display at (23, 1) : "  PRESS ANY KEY TO BEGIN."

# Title screen loop

170	! Use sprites to display redefined characters
180	for a = 97 to 122
190		!
200		! Loop for testing key presses
210		call color(#a - 96, 2)
220		for b = 1 to 10
230			call key(0, k, s) ::
			if s <> 0 then 340
240		next b

250		call color(#a-96, 12)

# There must have been a system limit of 24 cases to a switch?

260		if a - 96 > 24 then 290
270			on a - 96 gosub 530, 550, 560, 580, 590, 610, 620, 640, 660, 670, 690, 700, 720, 730, 740, 750, 760, 770, 780, 800, 810, 820, 840, 860
280		goto 300
290			on a - 96 - 24 gosub 880, 900

300		call sprite (#1, a, 2, 192 / 2, 196 / 2 + 24)
310	next a

320	call say("PRESS ANY KEY TO START")::
	goto 180

# Title screen finished

330	! Key was pressed. Delete sprites and goto list of words.
340	call clear ::
	call delsprite(all) ::
	goto 940

# Title screen

350	display at(2, 1) : "    Little Letters-           Do they Matter?" : : :
	                   "    By Eric Fischer" : 
	                   "    (Grade 5)

360	call vchar(12, 17, 143, 3) ::
	call vchar(12, 16, 143, 3)

370	for a = 1 to 26
380		!
390		call sound(100, 440 - a * 11 + 110, 5)
400		call sprite(#a + 1, 96 + a, 192 / 2, 196 / 2 + 24, int(40 * rnd) - 20, int(40 * rnd) - 20)
410	next a
420	return

# "Music" for title screen

430	! Music for title screen
440	for a = 110 to 440 step 10
450		call sound(a / 2, a, 0)
460	next a

470	for a = 1 to 5
480		call sound(220, 440, a * 2)
490	next a

500	call sound(1030, 110, 0)
510	return

# Unused?

520	call clear

# Sound for A

530	call sound(1000, 262, 0, 196, 0, 165, 0)
540	return

# Sound for B

550	goto 530

# Sound for C

560	call sound(1000, 392, 0, 196, 0, 165, 0)
570	return

# Sound for D

580	Goto 560

# Sound for E

590	call sound(1000, 440, 0, 220, 0, 175, 0)
600	return

# Sound for F

610	goto 590

# Sound for G

620	call sound(1500, 392, 0, 196, 0, 165, 0)
630	return

# Sound for G

640	call sound(1000, 349, 0, 220, 0, 175, 0)
650	return

# Sound for H

660	goto 640

# Sound for I

670	call sound(1000, 330, 0, 196, 0, 165, 0)
680	return

# Sound for J

690	goto 670

# Sound for K

700	call dound (750, 294, 0, 196, 0, 175, 0)
710	return

# Sound for L

720	goto 700

# Sound for M

730	goto 700

# Sound for O

740	goto 700

# Sound for P

750	goto 530

# sound for Q

760	goto 560

# sound for R

770	goto 560

# sound for S

780	call sound(1500, 349, 0, 220, 0, 175, 0)
790	return

# sound for T

800	goto 670

# sound for U

810	goto 670

# sound for V

820	call sound(1500, 294, 0, 196, 0, 175, 0)
830	return

# sound for W

840	for w = 1 to 3 ::
		cal sound (500, 392, 0, 196, 0, 16, 0) ::
	next w
850	return

# sound for X

860	call sound(1000, 294, 0, 175, 0, 196, 0)
870	return

# sound for Y

880	call sound(1000, 294, 0, 175, 0, 196, 0)
890	return

# sound for Z

900	!
910	!
920	call sound(2000, 262, 0, 165, 0, 196, 0)
930	return

# Retrieve words

# Did the DATA section actually have to be executed,
# or did I just think that it did?

# This word list must come from the same original source as
# http://www.bsacramentschool.com/yahoo_site_admin/assets/docs/Kindergarten_Review_for_Final_Exam.13672523.pdf
# Appears to be the "Dolch Sight Word Listing" by Edward Dolch

940	call clear
950	data a, and, away, big, blue, can, come, down, find, for, funny, go, help, here, I, in, is, it, jump, little
960	data look, make, me, my, not, one, play, red, run, said, see, the, three, to, two, up, we, where, yelllow, you

970	! both
980	data A, AND, AWAY, BIG, BLUE, CAN, COME, DOWN, FIND, FOR, FUNNY, GO, HELP, HERE, I, IN, IS, IT, JUMP, LITTLE
990	data LOOK, MAKE, ME, MY, NOT, ONE, PLAY, RED, RUN, SAID, SEE, THE, THREE, TO, TWO, UP, WE, WHERE, YELLOW, YOU

1000	! CAPS
1010	data all, am, are, at, ate, be, black, brown, byt, came, did, do, eat, four, get, good, have, he, into, like
1020	data must, new, no, now, on, our, out, please, pretty, ran, ride, saw, say, she, so, soon, that, there, they, this

1030	! both
1040	
