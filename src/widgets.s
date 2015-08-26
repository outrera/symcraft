use util gui

Skin = No
Skins = t
SkinCache = No
FontCache = No
FontTints = No

set_skin Path =
| Skins.Skin <= [SkinCache FontCache FontTints]
| Skin <= Path
| when got!it Skins.Skin:
  | SkinCache <= it.0
  | FontCache <= it.1
  | FontTints <= it.2
  | leave 0
| SkinCache <= t
| FontCache <= t
| FontTints <= "[Skin]/font/tints.txt"^cfg{}.map{[?0 ?.tail.pad{256 #FF000000}^new_cmap]}.table

skin F = have SkinCache.F: gfx "[Skin]/[F].png"

skin_cursor F =
| F = "cursor/[F]"
| have SkinCache.F: leave
  | Gfx = skin F
  | Gfx.xy <= "[Skin]/[F].txt".get.utf8.parse.0
  | Gfx

type font{@new_font Gs W H} glyphs/Gs widths/W height/H
font.as_text = "#font{}"
font N = have FontCache.N:
| Path = "[Skin]/font/[N]"
| G = gfx "[Path].png"
| [W H] = "[Path].txt".get.utf8.parse.0
| Glyphs = G.frames{W H}
| Ws = Glyphs{[X Y W H].margins=>X+W}
| Ws.0 <= W/2
| new_font Glyphs Ws H
font.width Line = | Ws = $widths; Line{C.code => Ws.(C-' '.code)+1}.sum
font.draw G X Y Tint Text =
| Ls = Text.lines
| Palette = FontTints.Tint
| when no Palette: bad "undefined font tint `[Tint]`; check tints.txt"
| Ws = $widths
| Gs = $glyphs
| H = $height
| CodePoint = ' '.code
| CY = Y
| for L Ls
  | CX = X
  | for C L
    | I = C.code-CodePoint
    | W = Ws.I
    | G.blit{CX CY Gs.I.recolor{Palette}}
    | W+1+!CX
  | !CY + H

type txt.widget{Value size/small tint/white}
     w h value_ size/Size tint/Tint font
| $font <= font $size
| $value <= Value
txt.draw G X Y = $font.draw{G X Y $tint $value_}
txt.as_text = "#txt{[$value]}"
txt.value = $value_
txt.`!value` Text =
| Text <= "[Text]"
| when $value_ >< Text: leave 0
| $value_ <= Text
| F = $font
| $w <= $value_.lines{}{L => F.width{L}}.max
| $h <= F.height

type bar.widget{V} value_/V.clip{0 100} bg/No
bar.render =
| have $bg: skin."bar/bg"
| Me
bar.value = $value_
bar.set_value New = $value_ <= New.clip{0 100}
bar.draw G X Y =
| G.blit{X Y $bg}
| G.rectangle{#347004 1 P+[3 3] [152*$value_/100 14]}

type button.widget{Text Fn state/normal w_size/large h_size/medium}
  value/Text on_click/Fn state/State over w_size/W_size h_size/H_size skin/No cache/(t)
button.reskin =
| Cache = $cache
| $skin <= Skin
| when got Cache.Skin: leave 0
| WSize = $w_size
| HSize = $h_size
| Text = $value
| Cache.Skin <= @table: map N [normal over pressed disabled]: list N
  | File = "button/[HSize]-[WSize]-[case N over normal _ N]"
  | G = File^skin.copy
  | P = case N pressed 2 _ 0
  | Tint = case N pressed+over | \white
                  disabled | \gray
                  _ | \yellow
  | F = font HSize
  | FW = F.width{Text}
  | FH = F.height
  | X = G.w/2-FW/2+P
  | Y = G.h/2-FH/2+P
  | F.draw{G X Y Tint Text}
  | G
button.render =
| when $skin <> Skin: $reskin
| State = $state
| when State >< normal and $over: State <= \over
| $cache.Skin.State
button.input In = case In
  [mice over S P] | $over <= S
  [mice left 1 P] | case $state normal: Me.state <= \pressed
  [mice left 0 P] | case $state pressed
                    | when $over: $on_click{}{}
                    | $state <= \normal
button.as_text = "#button{[$value]}"

type litem.widget{Text w/140 state/normal}
  text_/Text w/W h state/State font fw fh init
litem.render =
| less $init
  | $h <= "litem/normal"^skin.h
  | $font <= font small
  | $fw <= $font.width{$text_}
  | $fh <= $font.height
  | $init <= 1
| Me
litem.text = $text_
litem.`!text` Text =
| $init <= 0
| $text_ <= Text
litem.draw G PX PY =
| BG = "litem/[$state]"^skin
| G.blit{P.0 P.1 BG.rect{0 0 $w BG.h}}
| Tint = case $state picked(\white) disabled(\gray) _(\yellow)
| X = 2
| Y = BG.h/2-$fh/2
| $font.draw{G PX+X PY+Y Tint $text_}
litem.input In = case In
  [mice left 1 P] | $state <= case $state normal(\picked) picked(\normal) X(X)

type droplist.widget{Xs w/140} w/W h/1 y ih xs/[] drop rs picked over above_all p
| less Xs.size: Xs <= [' ']
| $xs <= Xs{(litem ? w/$w)}
droplist.text = $xs.$picked.text
droplist.`!text` T = $xs.$picked.text <= T
droplist.render =
| $rs <= map X $xs X.render
| case $rs [R@_]: $ih <= R.h
| when $drop
  | $y <= -$ih*$picked
  | $h <= $ih*$rs.size
| less $drop
  | $y <= 0
  | $h <= $ih
| Me
droplist.draw G PX PY =
| when $drop
  | Y = 0
  | for R $rs
    | G.blit{PX PY+Y R}
    | !Y + R.h
| less $drop
  | G.blit{PX PY $rs.$picked}
  | A = skin "arrow/down-normal"
  | G.blit{PX+$w-A.w PY A}
| $rs <= 0
| No
droplist.input In = case In
  [mice over S P] | $over <= S
                  | $xs.$p.state <= case S 1(\picked) 0(\normal)
  [mice_move _ P] | when $drop
                    | $xs.$p.state <= \normal
                    | $p <= ((P.1+$picked*$ih)/$ih).clip{0 $xs.size-1}
                    | $xs.$p.state <= \picked
  [mice left 1 P] | $drop <= 1
                  | $p <= $picked
                  | $above_all <= 1
                  | $xs.$p.state <= \picked
  [mice left 0 P] | $drop <= 0
                  | $xs.$p.state <= \normal
                  | $picked <= $p

type litems.$box{Xs w/300 lines/5 f/(V=>)} f/F ih/No lines/Lines xs/Xs box picked o/No
| $box <= layV: dup $lines: litem '' w/W
| $offset <= 0
litems.offset = $o
litems.`!offset` NO =
| when NO >< $o: leave 0
| $o <= max 0: @clip 0 $xs.size-1 NO
| times K $lines
  | I = $o + K
  | Item = $box.items.K
  | if I < $xs.size
    then | Item.text <= $xs.I
         | Item.state <= if I >< $picked then \picked else \normal
    else | Item.text <= ''
         | Item.state <= \disabled
litems.value = $xs.$picked
litems.data = $xs
litems.`!data` Ys =
| $xs <= Ys
| $picked <= 0
| $o <= No
| $offset <= 0
litems.input In = case In
  [mice left 1 P] | have $ih: $box.items.0.render.h
                  | NP = @clip 0 $xs.size-1 P.1/$ih+$o
                  | when NP <> $picked
                    | K = $picked - $o
                    | when K >> 0 and K < $lines:
                      | $box.items.K.state <= \normal
                    | $picked <= NP
                    | $f $xs.NP
                    | $box.items.(NP-$o).state <= \picked
litems.itemAt Point XY WH = [Me XY WH] //override lay's method

folder_nomalized Path = 
| Xs = Path.items
| Folders = Xs.keep{is."[_]/"}
| Files = Xs.skip{is."[_]/"}
| Parent = if Path >< '/' or Path.last >< ':' then [] else ['../']
| [@Parent @Folders @Files]

type slider_.widget{D f/(N=>) size/124 value/0.0 state/normal delta/No}
     dir/D f/F size/Size pos/0.0 state/State skin/No w/1 h/1 delta/Delta
| have $delta (10.0/$size.float)
| $value <= Value
slider_.value = $pos/$size.float
slider_.`!value` V =
| OV = $value
| $pos <= (V*$size.float).clip{0.0 $size.float}
| when $value <> OV: $f $value
slider_.render =
| S = skin "slider/[$dir]-normal"
| $w <= S.w
| $h <= S.h
| if $dir >< v then $h <= $size else $w <= $size
| Me
slider_.inc = !$value + $delta
slider_.dec = !$value - $delta
slider_.draw G PX PY =
| BG = skin "slider/[$dir]-normal"
| K = skin "slider/knob"
| I = 0
| when $dir >< v
  | while I < $size
    | G.blit{PX PY+I BG.rect{0 0 BG.w (min BG.h $size-I)}}
    | !I + BG.h
  | G.blit{PX+1 PY+$pos.int*($size-K.h)/$size+1 K}
| when $dir >< h
  | while I < $size
    | G.blit{PX+I PY BG.rect{0 0 (min BG.w $size-I) BG.h}}
    | !I + BG.w
  | G.blit{PX+$pos.int*($size-K.w)/$size+1 PY+1 K}
slider_.input In = case In
  [mice_move _ P] | when $state >< pressed
                    | NP = @clip 0 $size: if $dir >< v then P.1 else P.0
                    | when NP <> $pos.int
                      | $pos <= NP.float
                      | $f $value
  [mice left 1 P] | when $state >< normal
                    | $state <= \pressed
                    | $input{mice_move P P}
  [mice left 0 P] | when $state >< pressed: $state <= \normal

type arrow.widget{D Fn state/normal} direction/D on_click/Fn state/State
arrow.render = skin "arrow/[$direction]-[$state]"
arrow.input In = case In
  [mice left 1 P] | when $state >< normal
                    | $state <= \pressed
                    | Repeat = => when $state >< pressed
                                  | $on_click{}{}
                                  | 1
                    | (get_gui).add_timer{0.25 Repeat}
  [mice left 0 P] | when $state >< pressed
                    | $on_click{}{}
                    | $state <= \normal
arrow.as_text = "#arrow{[$direction] state([$state])}"

slider D @Rest =
| S = slider_ D @Rest
| if D >< v
  then layV [(arrow up (=>S.dec)) S (arrow down (=>S.inc))]
  else layH [(arrow left (=>!S.dec)) S (arrow right (=>S.inc))]

type folder_litems.$litems{Root f/(V=>)} root/Root f/F litems
| when $root.last <> '/': $root <= "[$root]/"
| $litems <= litems lines/9 f/(N => F "[$root][N]") $root^folder_nomalized
folder_litems.input In = case In
  [mice double_left 1 P] | R = if $litems.value >< '../'
                               then "[$root.lead.url.0]"
                               else "[$root][$litems.value]"
                         | when R.folder
                           | $root <= R
                           | $litems.data <= $root^folder_nomalized
  Else | $litems.input{In}
folder_litems.itemAt Point XY WH = [Me XY WH]

folder_widget Root F =
| FL = folder_litems Root f/F
| S = slider size/124 v f/(N => FL.offset <= @int N*FL.data.size.float)
| layH [FL S]

type minimap.widget{Main CenterAt} main/Main w/128 h/128 pressed center/CenterAt
minimap.draw G PX PY =
| MM = $main.world.minimap
| [X Y] = $main.world.player.view/32
| X = X*$w/$main.world.w
| Y = Y*$h/$main.world.h
| W = $main.view_w/32*$w/$main.world.w
| H = $main.view_h/32*$h/$main.world.h
| G.blit{PX PY MM}
| G.rectangle{#A0A0A0 0 PX+X PY+Y W H}

minimap.center_at P = ($center){[P.0*$main.world.w/$w P.1*$main.world.h/$h]}

minimap.input In = case In
  [mice_move _ XY] | when $pressed: $center_at{XY}
  [mice left 1 XY] | $pressed <= 1; $center_at{XY}
  [mice left 0 XY] | $pressed <= 0

type img.widget{Path} path/Path
img.render = skin $path

type icon_popup.widget info enabled resources text/txt{''}
| $info <= layV: map X [$text]: tabs 0: t 1(X) 0(spacer 0 0)
icon_popup.render =
| for X $info.items: X.pick{$enabled}
| $info.render

type icon.widget{data/0 click/(Icon=>)}
   w/50 h/42 pressed over fg tint g/skin{'icon/frame'}.copy
   data/Data on_click/Click popup/icon_popup{} last_fg last_tint
icon.draw G PX PY =
| less $tint: leave 0
| when $fg^address <> $last_fg^address or $tint^address <> $last_tint^address:
  | $g.blit{2 2 $fg.recolor{$tint}}
| when $pressed:
  | !PX + 1
  | !PY + 1
| G.blit{PX PY $g}
| when $over: G.rectangle{#A0A0A0 0 PX-2 PY-2 54 46}
| $last_fg <= $fg
| $last_tint <= $tint

icon.input In = case In
  [mice over S P] | $over <= S
  [mice left 1 P] | less $pressed: $pressed <= 1
  [mice left 0 P] | when $pressed:
                    | when $over: $on_click{}{Me}
                    | $pressed <= 0

type icon_hp.widget unit w/52 h/7 font/font{tiny}
icon_hp.draw G PX PY =
| less $unit: leave 0
| G.rectangle{#000000 1 PX PY $w $h}
| N = $unit.hp_percent
| C = if N < 50 then #F00000
      else if N < 75 then #F0F000
      else #00A000
| G.rectangle{C 1 PX PY N*$w/100 $h}
| HP = "[$unit.hp-$unit.hits]/[$unit.hp]"
| FW = $font.width{HP}
| $font.draw{G PX+($w-FW)/2+1 PY white HP}


export set_skin skin_cursor skin font txt button droplist slider folder_widget
       minimap img icon icon_hp
