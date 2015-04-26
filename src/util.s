use macros

MCs = | C = [water land plain air forest wall rock dead invuln 0 1 2 3 4 5 6 7 8 9 10]
      | (C.i){[?1 2**?0]}.table

Dirs = 8{(?.float-2.0)*PI/4.0}{[?.cos ?.sin].round.int}
dirN V = Dirs.locate{V.sign}

cfg File =
| less File.exists: bad "cant open [File]"
| File.get.utf8.parse{src File}.skip{is.[]}

list.points =
| [X Y W H] = Me
| dup I W*H [X+I%W Y+I/W]

list.overlaps [BX BY BW BH] =
| [AX AY AW AH] = Me
| AX<BX+BW and AY<BY+BH and BX<AX+AW and BY<AY+AH

export cfg Dirs dirN MCs
