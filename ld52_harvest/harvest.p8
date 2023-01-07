pico-8 cartridge // http://www.pico-8.com
version 39
__lua__
#include code/harvest.lua


__gfx__
000000003333333333333333333333334444444433355544444444335dddddd555dddddd5dddddd5555555551111121111111211115151515151515151515151
00000000333333333433334333433333444444443554444444444443d55555555555555555555555555555551111111221111111155555555555555555555555
00700700333333333333333333334333444444445444444444244443d5555ddddddd555555555d5555d555551221211221122112555555555555555555555555
00077000333333333333433333333333444444445444924449444444d55555ddddd5555555555555555555551221111111112122155555555555555555555555
00077000333333333433333334333333444444445444444444444444d555555ddd55555555555555555555551121111121111122555555555555555555555555
00700700333333333334333334433333444444445444944444444444d5555d55d55d555555d55d5555d55d551111111111111111155555555555555555555555
00000000333333333433334334433433444444445444444449444444d5555dd555dd555555555555555555551111112111221111555555555555555555555555
00000000333333333333333334333333444444444444924444244444d5555ddddddd555555555555555555551221111111221112155555555555555555555555
00077770000000000000000000000000000000004444444444444444d55555ddddd5555555d55d55d5d555551221211111111111555555555151515153333333
0077aaa9000000000000000000000000000000004444444444444449d555555ddd55555555555555555555551111111121111221155555553333333313333333
007a99a9000000000000000000000000000000004444944449244449d5555d55d55d555555555555555555551221211111111221555555553333333353333333
007a9aa9000000000000000000000000000000004444424444244449d5555dd555dd555555555d5555d555551221111111211111155555553333333313333333
007a9aa9000000000000000000000000000000004444444444444493d5555ddddddd555555555555555555551111111111111111555555553333333353333333
007a9aa9000000000000000000000000000000003444944449444493d55555ddddd5555555555555555555551112211112221121155555553333333313333333
007aaa99000000000000000000000000000000003344924444244933d555555ddd55555555555555555555551111112111122121555555553333333353333333
00099990000000000000000000000000000000003344444444449333d5555555d55555555dddddddddddddd52122221211211111155555553333333313333333
000000000000000000000000515151513344b33344444333433b43335dddddd555dddddd5dddddd555dddddd5555555555555555555555555333333333333333
00000000000000000000000000000000b44944444494444444434443d555555555555555d5555555555555555555555555595555155555551333333333333333
0000000000000000000000000000000044411111b394454444449944d5555ddddddd5555d5555dddddd5555555555555559995555555555553333b333b333333
00000000000000000000000000000000944455555114111143444944d55555ddddd55555d55555ddddd55555555555555999995515555555133b3b3b33333b33
00000000000000000000000000000000955555555544155541551433d555555ddd555555d555555ddd555555559999955555555555555555533b333b33333333
00000000000000000000000000000000941555d5555555554555494bd5555d55d55d5555d5555dd5d5d555555559995555555555155555551333333333333333
00000000000000000000000000000000444155555555555555554943d55d5dd555dd5555d55d5ddd5dd5d555555595555555555555555555533333333b333333
0000000000000000000000000000000044545555555555555555544455dd5ddddddd555555dd5dddddd5dd555555555555555555155555551333333333333333
000000000000000050000000500000009455555555555555555551945ddd55ddddd555555ddd55ddddd5ddd55555555555555555555555555555555555555555
000000000000000050000000100000004455d55555555d5555555534ddddd55ddd555555ddddd55ddd55dddd5559555555955955555595555555555566666666
000000000000000050000000500000004155555555555555555555345ddddd55555555555ddddd55555dddd55599555559999995555599555555555551515515
0000000000000000500000001000000044555555555555555555553155dddddd5555555555dddddd5ddddd5559995555999999995555999555555555b115151b
00000000000000005000000050000000455d555555555555555d5513d55d555555555555d55d55555555d55555995555599999955555995555555555b151511b
00000000000000005000000010000000455555555555555555555553d555555555555555d55555555555555555595555559559555555955555555555b111151b
0000000000000000500000005000000045555b55535555555b555553d555555555555555d55555555555555555555555555555555555555555555555b515111b
00000000000000005000000010000000435353355b55535b533553b3d555555555555555d55555555555555555555555555555555555555555555555b111151b
00000000007700000000000007777000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000007470000000000007bbb7000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000744777000000000073370000000000000330000000000000033000000000000003300000000000000330000000000000033000000000000000000000
00000077743337000000077773377700000000000330000000000000033000000000000003300000000000000330000000000000033000000000000000000000
000077788333300000007a9933999970000003300330000000000330033000000000033003300000000003300330000000000330033000000000000000000000
00078888833387000007aa9999999947000073337377000000007333737700000000733373770000000073337377000000007333737700000000000000000000
00788ee8888888700079a994a99999470007ee33e3ee70000007cc33c3cc70000007dd33d3dd70000007ff33f3ff700000071133131170000000000000000000
0078eee888888270007aa999a94a9947077eeeeeeeeee770077cccccccccc770077dddddddddd770077ffffffffff77007711111111117700000000000000000
0078ee8888888270007a9999a99a999707eeeeeeeeeeee7007cccccccccccc7007dddddddddddd7007ffffffffffff7007111111111111700000000000000000
0078888888888270007a9994a99aa9477eeeeeeeeeeeeee77cccccccccccccc77dddddddddddddd77ffffffffffffff771111111111111170000000000000000
0007888888888270007a9999a949a9477eeeeeeeeeeeeee77cccccccccccccc77dddddddddddddd77ffffffffffffff771111111111111170000000000000000
0007888888882270007a9994a999a9477eeeeeeeeeeeeee77cccccccccccccc77dddddddddddddd77ffffffffffffff771111111111111170000000000000000
0007888888822270007a999aa994a99707eeeeeeeeeeee7007cccccccccccc7007dddddddddddd7007ffffffffffff7007111111111111700000000000000000
000078888822270000799949a9949947077eeeeeeeeee770077cccccccccc770077dddddddddd770077ffffffffff77007711111111117700000000000000000
000007822227700000077799944494700007eeeeeeee70000007cccccccc70000007dddddddd70000007ffffffff700000071111111170000000000000000000
00000077777000000000007777777700000077777777000000007777777700000000777777770000000077777777000000007777777700000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000777777700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077666676677000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766666766766700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00755667667655700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00777575555577700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00755777777766700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766666666666700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766668886666700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766688888665700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766688888665700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00766668886555700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00755666665665700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00077555555577000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000777777700000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
09999999202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
99999999000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000ddddddddd0000000ddddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000dddddddd00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000ddddddd000000dddddddddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000dd000dd0000000dd0000ddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000ddd000000000000000ddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000d0ddd0d0000000000000ddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000d0ddd0d0000000000dddddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000ddd00000000000dddddd00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000dd000dd000000000dddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000ddddddd0000000000dd00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000ddddddddd000000000dd00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
000000000000000000000dddd0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0000000000000000000000dd00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
20202020202020202020202020202020000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
__map__
0101010101010101010101010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101010101010101010101010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101010101010101020101010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101050605060105060506010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101151615160115161516010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0102050605060105060506010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0102151615160115161516010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0102050605060105060506010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101151615160115161516010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101010101010101010101010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0124252601242526012425260101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0134353601343536013435360101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101010101020101010102010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101012425260124252601010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101013435360134353601010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0101010101010101010101010101010100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3e1d3e3e1d3e3e1d3e3e1d3e3e2e000d3e0f330d3e0f330d3e0f330d2c0f330000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3e3e3e3e3e3e3e3e3e3e3e3e3e1f003e3e3e323b3e3e323e3e3d323e3e3e320000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
2d2b3e2d2b3e2d2b3e2d2b3e2d2b3e1f002d2b3e332d3e3e332d3e3e332d3e3e330000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0d3e0f0d3e0f0d3e0f0d3e0f0d3e0f1f00232323002323230023232300232323000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3e3e3e3e3e3e3e3e3e3e3e3e3e1f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
2d2b3e2d2b3e2d2b3e2d2b3e2d2b3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0d3e0f0d3e0f0d3e0f0d3e0f0d3e0f1f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3e3e3e3e3e3e3e3e3e3e3e3e3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
2d2b3e2d2b3e2d2b3e2d2b3e2d2b3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0d3e0f0d3e0f0d3e0f0d3e0f0d3e0f1f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3d3e3e3e3e3e3e3e3e3e3b3e3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
2d3e3e2d2b3e2d2b3e2d2b3e2d3e3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
0d3e0f0d3e0f0d3e0f0d3e0f0d3e0f1f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1d3e3d3e3e3d3e3e3e3b3e3e3b3e3e1f00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
2d3e3e2d3e3e2d2b3e2d3e3e2d3e3e2e00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
1e3f1e1e3f1e1e3f1e1e3f1e1e3f1e0200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
