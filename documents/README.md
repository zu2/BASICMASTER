# Hitach BASICMASTER Document

気がついたことをメモします。

## Monitor $F00F (KBIN)

MB-6881のマニュアルではキーボードが推されているとC=1と書かれているが、逆(C=0)。
MB-6885のマニュアルでは修正されている

## I/O port $EFD0

MB-6885のROM/RAM切り替え。回路図IC6の74LS138P(3-Line To 8-Line Decoders)で$40間隔でデコード。
74LS138PのピンY7($EFC0)とアドレスバスA4のANDを取って$EFD0にしている(IC13)。
（同じくA5とのANDでグラフィックディスプレイ切り替え信号を生成している）

IC13のピン8(~ROM MASK)はIC12 74LS175M(Quadruple D-Type Flip-Flop)のCKに入り、データD0-2,4をラッチする。
- D0: ROM MASK1 B000-DFFF 1:RAM, 0:BASIC ROM
- D1: ROM MASK2 E000-EFFF 1:RAM, 0:PRINTER ROM, I/O
- D2: ROM MASK3 F000-FFFF 1:RAM, 0:MONITOR ROM
- D4: TIME OFF 1:ON 0:OFF

D4は回路図2/3につながっているはずだが、回路図が無い…
回路図2/3はブロックダイヤグラムから想像するに、システム系動機信号発生回路、画面表示、拡張端子あたりの回路が含まれていたと思われる)

bm2ではエミュレートされていない。ebmjr、j68はOK。MAMEは調査してない。

cf. http://cmpslv2.starfree.jp/Bmjr/EnrBm.htm

