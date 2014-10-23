use util

type unit
    id type xy disp owner color team name side hits mana
    frame dir/Dirs.0 resources/(t size/6)
    enemies nobody playable rescueable passive view
    world content_next sensor_next last_drawn/-1 mm_color seen
    act last_selected
heir unit $type
unit.as_text = "#unit{[$type.id]}"

unit.init_mm_color =
| $mm_color <= if $owner >< $world.this_player then #00FF00
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

unit.deploy P =
| $xy <= P
| $disp <= P*32
| $mark

unit.make_sound Sound =

unit.hp_percent = max 0 ($hp-$hits)*100/$hp

unit.alive = $hp-$hits > 0

unit.order What Type Target =
| say "ordering [Me] to [What] [Type] [Target]"

export unit
