use gui util widgets


type panel.$tabs{View} view/View unit unit_icon unit_name unit_hp unit_stats
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
                |  10  60 | layH [$prod_txt $prod_icon]
| $tabs <= tabs none: t
  none    | spacer 1 1
  normal  | dlg [@Base @Normal]
  produce | dlg [@Base @Produce]
| Spacer = spacer 50 42
| Click = Icon => View.pick_target{Icon.data}
| $act_icons <= dup 9: tabs 0: t 1(icon data/[0 0 0] click/Click) 0(Spacer)

act_types Types What Pref As =
| As{Types.?}.replace{No 0}.skip{(? and ?hide)}{[What Pref ?]}

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
  | $unit_name.value <= $unit.ui_name
  | $unit_stats.value <= $extract_stats{$unit}
  | $unit_hp.unit <= $unit
  | Ts = World.main.types
  | As = [@$unit.acts^act_types{Ts do 0}
          @$unit.trains^act_types{Ts train 'Train'}
          @$unit.morphs^act_types{Ts morph 'Upgrade to'}
          @$unit.researches^act_types{Ts 'research' 'Research'}]
  | when $unit.builds.size: As <= [@As [do 0 Ts.build_basic]]
  | when $unit.builds.size > 8: As <= [@As [do 0 Ts.build_advanced]]
  | for [I [What Pref Type]] As.pad{9 [0 0 0]}.i
    | Tabs = $act_icons.I
    | FG = Type and Type.icon.Side^~{0}
    | if FG
      then | Icon = Tabs.all.1
           | Icon.fg <= FG
           | Icon.tint <= Tint
           | ProdName = Type.prodName or Type.ui_name
           | Name = if Pref then "[Pref] [ProdName]" else ProdName
           | Icon.data.init{[$unit What Type]}
           | Icon.popup.text.value <= Name
           | Icon.popup.enabled <= 1
           | Tabs.pick{1}
      else | Tabs.pick{0}
  | $tabs.pick{normal}
| $tabs.render

panel.draw G X Y =
| $tabs.draw{G X Y}
| $unit_stats.value <= 0

panel.extract_stats U =
| Xs = ["Armor: [U.armor]"
        U.damage^| $No [B P] => when B or P: "Damage: [P/2]-[B+P]"
        "Range: [U.range]"
        "Sight: [U.sight]"
        (when U.speed "Speed: [U.speed]")
        (when U.supply "Supply: [U.supply]")
        (when U.mp "Mana: [U.mana]/[U.mp]")]
| Xs.skip{No}.text{'\n'}


type view.widget{W H M} g w/W h/H main/M player paused/1 last_click
                        a units notes speed/20 frame pointer selection
                        target_blink keys/(t) mice_xy/[0 0] anchor
                        panel act
| $g <= gfx W H
| $panel <= panel{Me}
| $panel.unit_icon.on_click <= (Icon => $center_on_selection)

view.init =
| $paused <= 0
| $player <= $world.player
| $selection <= []
| $target_blink <= [0 0 0]
| $act <= [0 0 0]
| $units <= 0
| $notes <= []
| $clear_clicks
| $move{$xy} //normalize view

view.pick_target As<[Actor What Type] =
| Target = Actor
| case What
  do | case Type.targets
       self | Actor.order{What Type Target}
       Else | $act.init{As}
  train+research+morph | Actor.order{What Type Target}
  Else | bad "cant [What]"

view.center_on_selection =
| $selection^|$0 [U@_] => $center_at{U.xy+U.size/2}

view.world = $main.world

view.clear_clicks = $last_click <= [[No 0] 0]

view.notify Player Text life/6.0 =
| when Player >< $player
  | $notes <= [@$notes [(clock)+Life Text]]

view.draw_unit U =
| Fr = $frame
| when U.last_drawn >< Fr: leave 0
| U.last_drawn <= Fr
| G = $g
| TN = $world.tileset_name
| SO = $xy
| Id = U.id
| Col = $world.tints.(U.color)
| D = dirN U.dir
| UG = U.sprite.TN.(U.frame).D
| [X Y] = U.disp - SO + U.size*16
| Rect = 0
| RC = 0
| Blink = 0
| [Target BT1 BT2] = $target_blink
| when U >< Target
  | Cycle = $world.cycle
  | Blink <= Cycle < BT1 or (Cycle > BT2 and Cycle < BT2 + 12)
| when U.last_selected >< Fr or Blink:
  | [SW SH] = U.selection
  | RX = X - SW/2
  | RY = Y - SH/2
  | Rect <= [RX RY SW SH]
  | RC <= U.mm_color
| when Rect and not U.building: G.rectangle{RC 0 @Rect}
| when D > 4 and not U.building: UG.flop
| G.blit{X-UG.w/2 Y-UG.h/2 UG.recolor{Col}}
| when Rect and U.building: G.rectangle{RC 0 @Rect}

view.xy = $player.view

view.move NewXY =
| XY = $xy
| XY.init{NewXY}
| [X Y] = XY
| WW = $world.w*32
| WH = $world.h*32
| XY.init{[X.clip{0 WW-$g.w} Y.clip{0 WH-$g.h}]}

view.center_at XY = $move{XY*32-[$w $h]/2}

view.render =
| G = $g
| W = $world.w
| H = $world.h
| Cs = $world.units
| TN = $world.tileset_name
| TP = $player
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
    | when!it C.content: push it Vs
    | G.blit{PX PY C.gfx}
    | PX += 32
    | X++
  | PY += 32
  | Y++
| for U $selection: U.last_selected <= $frame
| Vs = Vs{?^uncons{content_next}}.join.uniq
| Vs = Vs.sort{[?layer ?disp.1] < [??layer ??disp.1]}
| for X Vs.keep{?building}: $draw_unit{X}
| for X Vs.skip{?building}: $draw_unit{X}
| $units <= Vs^cons{seen}
| when $anchor
  | [X Y W H] = $mice_rect
  | when [W H].abs >> 10.0: G.rectangle{#00ff00 0 X Y W H}
| less $frame: (get_gui).add_timer{1.0/120.0 (=>$update)}
| $frame++
| get_gui{}.focus_widget <= Me //ensure we always have keyboard focus
| G

view.mice_to_cell XY =
| [X Y] = ($xy+XY)/32
| $world.get{X.clip{0 $world.w-1} Y.clip{0 $world.h-1}}

view.unit_rect U = [@(U.disp-$xy+U.size*16-U.selection/2) @U.selection]


view.input_select_single MiceXY =
| Us = $units^uncons{seen}
| Us = Us.keep{U => MiceXY.in{$unit_rect{U}}}
| Us.sort{?layer>??layer}^|$[] [U@_] => [U]

view.input_select =
| MR = $mice_rect
| if not $anchor or [MR.2 MR.3].abs < 10.0
  then $input_select_single{$mice_xy}
  else | Us = $units^uncons{seen}.skip{?building}
       | Us.keep{U=>$unit_rect{U}.overlaps{MR} and U.owner >< $player}

view.mice_rect =
| [AX AY] = if $anchor then $anchor else $mice_xy
| [BX BY] = $mice_xy
| X = min AX BX
| Y = min AY BY
| U = max AX BX
| V = max AY BY
| [X Y U-X V-Y]

view.pick_cursor =
| $pointer <= skin_cursor if $act.0 then \ch_red
                          else if $anchor then \cross
                          else if $input_select_single{$mice_xy}.size then \glass
                          else \point

view.ack Actor Target CrossCell =
| $clear_clicks

view.input In = case In
  [mice_move _ XY]
    | $mice_xy.init{XY}
    | $pick_cursor 
  [mice left 1 XY]
    | when $act.0: leave 0
    | $anchor <= XY
    | $pick_cursor 
  [mice left 0 XY]
    | $mice_xy.init{XY}
    | C = $mice_to_cell{$mice_xy}
    | if $act.0
      then | [Actor What Type] = $act
           | $anchor <= $mice_xy
           | Target = $input_select^|$C [U@_] => U
           | when Target.is_unit
             | $target_blink.init{[Target $world.cycle+12 $world.cycle+24]}
           | Actor.order{What Type Target}
           | $act.init{[0 0 0]}
      else | $selection <= $input_select
    | $anchor <= 0
    | $pick_cursor 
  [mice right 1 XY]
    | if $act.0
      then | $act.init{[0 0 0]}
           | $pick_cursor
      else
  [key up    1] | $move{$xy-[0 64]}
  [key down  1] | $move{$xy+[0 64]}
  [key left  1] | $move{$xy-[64 0]}
  [key right 1] | $move{$xy+[64 0]}
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
