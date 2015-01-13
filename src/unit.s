use util macros heap

type unit.$type
    id type xy/[0 0] disp/[0 0] owner color team name side hits mana
    on_map frame dir/Dirs.0 resources/(t size/6)
    enemies nobody playable rescueable passive view
    world active_next content_next sensor_next last_drawn/-1 mm_color seen
    last_selected parent content anim/[] anim_wait anim_index
    new_goal/[0 0 0] goal/[0 0 0] path/dup{PATH_CACHE_SIZE}
unit.as_text = "#unit{[$tid] [$id] [$xy]}"

unit.center = $xy + $size/2

unit.center_disp = $disp + $size*16

unit.distance_to Target =
| As = [@$xy @$size].points
| Bs = [@Target.xy @Target.size].points
| @int: @round: @min: map A As: @min: map B Bs: @abs B-A

unit.init_mm_color =
| $mm_color <= if $owner >< $world.player then #00FF00
               else $world.main.ui_colors.($color).0

unit.hash = $id
unit.`><` B = B.is_unit and $id >< B.id

unit.mark =
| S = $sight
| $world.upd_area{[@$xy @$size]
  | C => | $content_next <= C.content
         | C.content <= Me
         | !C.mask--$layer
| $world.upd_area{[@($xy-[S S]) @($size+[2*S 2*S])]
  | C => | $sensor_next <= C.sensors
         | C.sensors <= Me}

unit.stop = $new_goal.init{[do $world.main.types.stop Me]}

unit.deploy P =
| $xy.init{P}
| $disp.init{P*32}
| $on_map <= 1
| $stop
| $anim <= $anims.still
| $mark

unit.make_sound Sound =

unit.hp_percent = max 0 ($hp-$hits)*100/$hp

unit.alive = $hp-$hits > 0

unit.order What Type Target =
| less $new_goal.1.forced
  | say "ordering [Me] to [What] [Type] [Target]"
  | $new_goal.init{[What Type Target]}

unit.can_move_to P =
| M = $mask
| L = $layer
| for XY [@P @$size].points:
  | CM = $world.get{@XY}.mask
  | less @mask M CM and (@mask CM L) >< 0: leave 0
| leave 1


VisitCycle = 0

aStar Limit StartCell Found Heuristic CanMoveTo =
| !VisitCycle+1
| Q = heap //priority queue
| Q.push{0 [StartCell 0 0]}
| while !it Q.pop:
  | O = it.value
  | C = O.0
  | G = O.2
  | when Found C
    | leave O^(@r [C Prev @_] => if Prev then [C @Prev^r] else []).flip
  | when G < Limit
    | NG = G+1
    | for N C.neibs.keep{CanMoveTo}
      | when N.visited <> VisitCycle
        | Q.push{NG+Heuristic{N} [N O NG]}
        | N.visited <= VisitCycle
| 0

unit.move_to P =
| H = C =>
  | [X Y] = C.xy
  | X*X + Y*Y
| Path = aStar 1024 $world.$xy (C => C.xy >< P) H (C => $can_move_to{C.xy})
| say Path
| halt
//$anim <= $anims.move

unit.rotate_facing =
| when $building: leave 0
| F = Dirs.locate{$dir} + [-1 1].rand
| when F < 0: F <= 7
| when F > 7: F <= 0
| $dir <= Dirs.F

unit.update_anim =
| A = $anim.$anim_index
| !$anim_index + 1
| case A
  [Frame Wait @Move]
    | $frame <= Frame
    | $anim_wait <= Wait
    | case Move [V]: !$disp + |$dir*V
  rotate
    | less 100.rand: $rotate_facing
    | $update
  attack
    | No
    | $update
  ship_attack
    | No
    | $update
  Else | bad "invalid anim: [A]"

unit.update =
| when $anim_wait: leave !$anim_wait - 1
| when $anim_index < $anim.size: leave $update_anim
| less $new_goal.0: $stop
| $goal.init{$new_goal}
| [What Type Target] = $goal
| less Type.repeat: $stop
| when Type.range < $distance_to{Target}:
  | $move_to{Target.xy}
  | leave 0
| $anim <= $anims.(Type.show)
| $anim_index <= 0
| leave 0


export unit
