use gui util widgets


type panel unit unit_icon unit_name unit_hp unit_stats
           prod_bar prod_txt prod_icon act_icons tabs
| $unit_icon <= icon
| $unit_name <= txt ''
| $unit_hp <= icon_hp
| $unit_stats <= txt ''
| $prod_bar <= spacer 1 1
| $prod_txt <= spacer 1 1
| $prod_icon <= spacer 1 1
| Base = mtx |  1  0 | $unit_icon
             | 62  4 | $unit_name
             |  0 43 | $unit_hp
| Normal = mtx | 15 62 | $unit_stats
| Produce = mtx |   3 144 | $prod_bar
                |  44 148 | txt '% Complete'
                |  10  60 | lay h 0 [$prod_txt $prod_icon]
| $tabs <= tabs none: t
  none    | spacer 1 1
  normal  | dlg [@Base @Normal]
  produce | dlg [@Base @Produce]
| Spacer = spacer 50 42
| $act_icons <= dup 9: tabs 0: t 1(icon) 0(Spacer)

heir panel $tabs

panel.render =
| less $unit
  | for T $act_icons
    | T.pick{0}
  | $tabs.pick{none}
| when $unit
  | World = $unit.world
  | Side = $unit.owner.side
  | Tint = World.tints.($unit.color)
  | $unit_icon.fg <= $unit.icon.Side
  | $unit_icon.tint <= Tint
  | $unit_name.value <= $unit.typename
  | $unit_stats.value <= $extract_stats{$unit}
  | $unit_hp.unit <= $unit
  | Ts = World.main.types
  | As = [@$unit.acts @$unit.trains @$unit.morphs @$unit.researches]
  | when $unit.builds.size: [@!As build_basic]
  | when $unit.builds.size > 8: [@!As build_advanced]
  | for [I A] As{Ts.?}.replace{Void 0}.skip{(? and ?hide)}.pad{9 0}.i
    | T = $act_icons.I
    | FG = A and A.icon.Side^supply{0}
    | if FG
      then | Icon = T.all.1
           | Icon.fg <= FG
           | Icon.tint <= Tint
           | Icon.popup.text.value <= A.typename
           | Icon.popup.enabled <= 1
           | T.pick{1}
      else | T.pick{0}
  | $tabs.pick{normal}
| $tabs.render

panel.draw G P =
| $tabs.draw{G P}
| $unit_stats.value <= 0

panel.extract_stats U =
| Xs = ["Armor: [U.armor]"
        U.damage^| $Void [B P] => when B or P: "Damage: [P/2]-[B+P]"
        "Range: [U.range]"
        "Sight: [U.sight]"
        (when U.speed "Speed: [U.speed]")
        (when U.supply "Supply: [U.supply]")
        (when U.mp "Mana: [U.mana]/[U.mp]")]
| Xs.skip{Void}.text{'\n'}


type view.widget{W H M} g w/W h/H main/M paused/1 sel/[] last_click/[]
                        ack a visible notes speed/20 frame cursor selection
                        sel_blink/[0 0 0] keys/(t) mice_xy/[0 0] anchor
                        units/0 panel/panel{}
| $g <= gfx W H
| $panel.unit_icon.on_click <= (=> $center_on_selection)

view.center_on_selection =
| $selection^|$0 [U@_] => $center_at{U.xy+U.size/2}

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
| $units <= Vs^cons{(?.seen <= ??)}
| when $anchor
  | [X Y W H] = $mice_rect
  | when [W H].abs >> 10.0: G.rect{#00ff00 0 X Y W H}
| less $frame: (get_gui).add_timer{1.0/24.0 (=>$update)}
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
  then | Us = $units^uncons{?seen}
       | Us = Us.keep{U => $mice_xy.in{$unit_rect{U}}}
       | Us.sort{?layer>??layer}^|$[] [U@_] => [U]
  else | Us = $units^uncons{?seen}.skip{?building}
       | Player = $world.this_player
       | Us.keep{U=>$unit_rect{U}.overlaps{MR} and U.owner >< Player}

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

view.units_aggregate Xs = case Xs [U]: U

view.update =
| when $paused: leave 1
| $world.update
| when $selection
  | $panel.unit <= $units_aggregate{$selection.keep{?alive}}
| 1

export view
