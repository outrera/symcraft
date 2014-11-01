use util macros

type unit
    id type xy/[0 0] disp/[0 0] owner color team name side hits mana
    on_map frame dir/Dirs.0 resources/(t size/6)
    enemies nobody playable rescueable passive view
    world active_next content_next sensor_next last_drawn/-1 mm_color seen
    last_selected parent content anim/[] anim_wait anim_index
    new_goal/[0 0 0] goal/[0 0 0] path/dup{PATH_CACHE_SIZE}
heir unit $type
unit.as_text = "#unit{[$type.id] [$id] [$xy]}"

unit.center = $xy + $size/2

unit.center_disp = $disp + $size*16

unit.distance_to Target =
| As = [@$xy @$size].xy
| Bs = [@Target.xy @Target.size].xy
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
         | @xor !C.mask $layer}
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

unit.move_to Target = //$anim <= $anims.move

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
  [Frame Wait]
    | $anim_wait <= Wait
  [Frame Wait Move]
    | $anim_wait <= Wait
    | !$disp + |$dir*Move.0
  rotate
    | $rotate_facing
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
| when Type.id >< move
  | say "[$distance_to{Target}]/[Type.range]"
  | say Me
  | halt
| when $distance_to{Target} > Type.range:
  | $move_to{Target.xy}
  | leave 0
| $anim <= $anims.(Type.show)
| $anim_index <= 0
| leave 0


export unit
