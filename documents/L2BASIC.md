# Hitach BASICMASTER L2 BASIC Document

## L2 BASICの変数表現

2通り。-32767〜32767までの整数か、仮数4バイト・指数1バイトの浮動小数。演算時には仮数をさらに1バイト追加して演算精度を高めている。

仮数部、指数部ともに2の補数形式。仮数部はケチ表現。

8bit指数・1bit符号・31bitの仮数からなる。指数は2の補数。
指数部が$17のときは整数値で、仮数部2バイト目・3バイト目に値が入る。4バイト目は0。1バイト目は2バイト目から符号拡張されている。

![インターフェース 1980年2月号(No.33) ベーシックマスター拡張・汎用ラボオート装置の製作 P.83より引用](./BASICMASTER-L2BASIC-floating-point.png)

## 変数テーブル

$700-
変数名と変数の種別、変数の格納先の4バイトで表される。
配列の場合、格納先に配列の大きさも格納されている。

変数名1文字目1バイト(ASCII)・2バイト目の上位ニブルは変数種別、下位ニブルは変数2文字目(1-9)。変数が1文字の場合は$A。3・4バイト目が変数へのポインタ。未初期化の場合は0000。

変数種別は$00:数値変数、$10:1次元数値変数、$20:2次元変数。文字変数はおのおの$C0を足す。
未初期化時は数値は$70、文字変数は$F0になっている。

## BASIC中間言語

行番号2バイト+行長1バイト+中間言語…

- I/O 1982年10月号 p.266- 「Jr.ユーリティと解析」
	- BASIC中間言語表など

BASIC ROMの$B178から予約語表。予約後の直後に中間言語の2の補数が続く。
予約語表中の$09は空白があってもいい場所、表の最後は$00。

\$DFA3から文のジャンプテーブル（RUNから始まる）。
\$CF7Aから関数のジャンプテーブル(INKEY$から始まる）。


|  |0x    |1x       |2x   |3x   |4x     |5x  |6x|
|--|------|---------|-----|-----|-------|----|--|
|x0|      |ON       |OPEN |MON  |INKEY$ |COS |! |
|x1|RUN   |IF       |CLOSE|L#   |CURSOR$|TAN |  |
|x2|LIST# |FOR      |INPUT|CONT |LEFT$  |ATN |  |
|x3|LIST  |DEFFN    |PRINT|RNDM |RIGHT$ |EXP |  |
|x4|DEL   |NEW      |CALL |MU   |MID$   |LOG |  |
|x5|RESEQ |MONITOR  |POKE |R    |STR$   |SQR |  |
|x6|SEQ   |SIZE     |LET  |L    |SPC$   |PEEK|  |
|x7|GOTO  |END      |NEXT |S    |CHR$   |VAL |  |
|x8|GOSUB |CONTINUE |PLOT |C    |PAI    |LEN |  |
|x9|LOAD  |STOP     |GOS  |THEN |TIME   |ASC |  |
|xA|SAVE  |CLEAR    |GO   |GOTO |CURSOR |TAB |  |
|xB|MERGE |RETURN   |RET  |GOSUB|INT    |HEX |  |
|xC|VERIFY|RESTORE  |IN   |TO   |ABS    |MOD |  |
|xD|REM   |RANDOMIZE|PR   |STEP |SGN    |CUR$|  |
|xE|MUSIC |DIM      |?    |GOS  |RND    |CUR |  |
|xF|DATA  |READ     |CLR  |GO   |SIN    |!$  |  |
