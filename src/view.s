use gui util widgets

type view.widget{W H M} g w/W h/H main/M paused/1 sel/[] last_click/[]
                        ack a visible notes speed/20 frame cursor selection
                        sel_blink/[0 0 0] keys/(t) mice_xy/[0 0] anchor
                        units/0
| $g <= gfx W H

view.world = $main.world

view.clear_clicks = $last_click <= [[Void 0] 0]

view.notify Player Text life/6.0 =
| when Player >< $world.this_player
  | $notes <= [@$notes [(clock)+Life Text]]

view.draw_unit U =
| Fr = $frame
| when U.last_drawn >< Fr: leave 0
| U.last_drawn <= Fr
| G = $g
| TN = $world.tileset_name
| SO = $world.this_player.view
| [BId BT1 BT2] = $sel_blink
| Cycle = $world.cycle
| Blink = Cycle < BT1 or (Cycle > BT2 and Cycle < BT2 + 12)
| Id = U.id
| Col = $world.tints.(U.color)
| D = dirN U.dir
| UG = U.sprite.TN.(U.frame).D
| [X Y] = U.disp - SO + U.size*16
| [SW SH] = U.selection
| RX = X - SW/2
| RY = Y - SH/2
| when not U.building and U.last_selected >< Fr: G.rect{#00ff00 0 RX RY SW SH}
| G.blit{[X-UG.w/2 Y-UG.h/2] UG map(Col) flipX(D > 4 and not U.building)}
| when U.building and U.last_selected >< Fr: G.rect{#00ff00 0 RX RY SW SH}

view.normalize_view =
| SO = $world.this_player.view
| [X Y] = SO
| WW = $world.w*32
| WH = $world.h*32
| VXY = $world.this_player.view
| VXY.0 <= X.clip{0 WW-$g.w}
| VXY.1 <= Y.clip{0 WH-$g.h}

view.player_view = $world.this_player.view

view.center_at XY =
| $world.this_player.view <= XY*32-[$w $h]/2
| $normalize_view

view.render =
| $normalize_view
| G = $g
| W = $world.w
| H = $world.h
| Cs = $world.units
| TN = $world.tileset_name
| TP = $world.this_player
| Cycle = $world.cycle
//| Side = TP.side
| SO = TP.view
| [IX IY] = -(SO%32)
| [CX CY] = SO/32
| CW = @int: @ceil ($w.float+31.0)/32.0
| CH = @int: @ceil ($h.float+31.0)/32.0
| Vs = [] // visible units
| EX = @clip 0 W CX+CW
| EY = @clip 0 H CY+CH
| Y = CY
| PY = IY
| while Y < EY
  | X = CX + Y*W
  | EX = EX + Y*W
  | PX = IX
  | while X < EX
    | C = Cs.X
    | when!it C.content: [it@!Vs]
    | G.blit{[PX PY] C.gfx}
    | !PX + 32
    | !X + 1
  | !PY + 32
  | !Y + 1
| for U $selection: U.last_selected <= $frame
| Vs = Vs{@r$[] V<-&0 => [V @V.content_next^r]}.join.uniq
| Vs = Vs.sort{[?layer ?disp.1] < [??layer ??disp.1]}
| for X Vs.keep{?building}: $draw_unit{X}
| for X Vs.skip{?building}: $draw_unit{X}
| $units <= Vs.cons{(?.seen <= ??)}
| when $anchor
  | [X Y W H] = $mice_rect
  | when [W H].abs >> 10.0: G.rect{#00ff00 0 X Y W H}
| !$frame + 1
| get_gui{}.focus_widget <= Me //ensure we always have keyboard focus
| G

view.mice_to_cell XY =
| [X Y] = ($player_view+XY)/32
| $world.get{X.clip{0 $world.w-1} Y.clip{0 $world.h-1}}

gather_content U = if U then [U @U.content_next^gather_content] else []

view.unit_rect U = [@(U.disp-$player_view+U.size*16-U.selection/2) @U.selection]

view.input_select =
| MR = $mice_rect
| if not $anchor or [MR.2 MR.3].abs < 10.0
  then | Us = $units.uncons{?seen}
       | Us = Us.keep{U => $mice_xy.in{$unit_rect{U}}}
       | Us.sort{?layer>??layer}^|$[] [U@_] => [U]
  else | Us = $units.uncons{?seen}.skip{?building}
       | Us.keep{U=>$unit_rect{U}.overlaps{MR}}

view.mice_rect =
| [AX AY] = if $anchor then $anchor else $mice_xy
| [BX BY] = $mice_xy
| X = min AX BX
| Y = min AY BY
| U = max AX BX
| V = max AY BY
| [X Y U-X V-Y]

view.pick_cursor =
| $cursor <= skin_cursor if $anchor then \cross
                         else if $mice_to_cell{$mice_xy}.content then \glass
                         else \point

view.input @In = case In
  [mice_move _ XY]
    | $mice_xy.init{XY}
    | $pick_cursor 
  [mice left 1 XY]
    | $anchor <= XY
    | $pick_cursor 
  [mice left 0 XY]
    | $mice_xy.init{XY}
    | CC = $mice_to_cell{$mice_xy}
    | $selection <= $input_select
    | $anchor <= 0
    | $pick_cursor 
  [key up    1] | !$player_view.1 - 64; $normalize_view
  [key down  1] | !$player_view.1 + 64; $normalize_view
  [key right 1] | !$player_view.0 + 64; $normalize_view
  [key left  1] | !$player_view.0 - 64; $normalize_view
  [key Name  S] | $keys.Name <= S

view.pause = $paused <= 1
view.unpause = $paused <= 0

export view
