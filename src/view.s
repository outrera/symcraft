use gui util widgets

type view.widget{W H M} g w/W h/H main/M paused/1 sel/[] last_click/[]
                        ack a visible notes speed/20 frame cursor
                        sel_blink/[0 0 0] keys/(t) mice_xy/[0 0] anchor
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
| G.blit{[X-UG.w/2 Y-UG.h/2] UG map(Col) flipX(D > 4 and not U.building)}

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
| Vs = Vs{@r$[] V<-&0 => [V @V.content_next^r]}.join
| Vs = Vs.sort{[?layer ?disp.1] < [??layer ??disp.1]}
| for X Vs.keep{?building}: $draw_unit{X}
| for X Vs.skip{?building}: $draw_unit{X}
| when $anchor
  | [X Y W H] = $selection_rect
  | when [W H].abs >> 10.0: G.rect{#00ff00 0 X Y W H}
| !$frame + 1
| get_gui{}.focus_widget <= Me //ensure we always have keyboard focus
| G

view.mice_to_cell XY =
| [X Y] = ($player_view+XY)/32
| $world.get{X.clip{0 $world.w} Y.clip{0 $world.h}}

view.input_select A B =
| Void /*if not $anchor then //$sel
          else if (A-B).abs >> 10.0
            then | R = //select
                 | selUnits*/

view.selection_rect =
| [AX AY] = if $anchor then $anchor else $mice_xy
| [BX BY] = $mice_xy
| X = min AX BX
| Y = min AY BY
| W = max AX BX
| H = max AY BY
| [X Y W-X H-Y]

view.input @In = case In
  [mice_move _ XY]
    | $mice_xy.init{XY}
    | CC = $mice_to_cell{$mice_xy}
    | less $anchor: $cursor <= skin_cursor if CC.content then \glass else \point
  [mice left 1 XY] | $anchor <= XY
                   | $cursor <= skin_cursor cross
  [mice left 0 XY]
    | $mice_xy.init{XY}
    | CC = $mice_to_cell{$mice_xy}
    | S = $input_select{$anchor $mice_xy}
    | $anchor <= 0
    | $cursor <= skin_cursor point
  [key up    1] | !$player_view.1 - 64
  [key down  1] | !$player_view.1 + 64
  [key right 1] | !$player_view.0 + 64
  [key left  1] | !$player_view.0 - 64
  [key Name  S] | $keys.Name <= S

view.pause = $paused <= 1
view.unpause = $paused <= 0

export view
