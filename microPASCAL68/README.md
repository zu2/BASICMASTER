# micro PASCAL 68/BM

月刊ASCII 1980年12月号に掲載されたmicro PASCAL 68のベーシックマスターL2/L2II/Jrへの移植です。 掲載誌のダンプリストとの差分を載せます。

## 移植方法

- オリジナルのダンプリストは$0100-14FFですが、これを$2100-$24FFまで入力します。
- ゼロページと絶対分岐の変更パッチ(patch.txt)を当てます
- $2000-$20FFにはBM用のI/Oルーチン(bmio.txt)を追加します
- $2100 (COLD), $2103(HOT)から実行してください

## work area

0ページのワーク領域は、アドレスに$A0を加えて利用。
MUSIC・浮動小数点演算ワーク領域を使っているので、音楽は出せない。

|orig addr	|BM addr	|
|-----------|-----------|
|$24-$53	|$C4-$F3	|

|orig	|BM addr	|name		|desc								|
|-------|-----------|-----------|-----------------------------------|
|$26	|$C6		|RNDV		|random reg.						|
|$28	|$C8		|SBADR		|proc jump table					|
|$2A	|$CA		|RBLEND		|top of array						|
|$2C	|$CC		|SKTOP		|top of local var					|
|$2E	|$CE		|VARAD/LBTBL|FOR var addr/proc table addr		|
|$30	|$D0		|EDINT/LOCAL|FOR end value/local table addr		|
|$32	|$D2		|LNO		|Error Line Number					|
|$33	|$D3		|WORK		|									|
|$34	|$D4		|PAS		|mode 0:edit,1:analyze,2:run,3:read	|
|$36	|$D6		|AENDMR		|memory end +1						|
|$38	|$D8		|STKTP		|top of stack						|
|$3A	|$DA		|BOFA		|buffer address						|
|$3C	|$DC		|FLEND		|end of file area					|
|$3E	|$DE		|EOF		|file end +1						|
|$40	|$E0		|PINT		|line pointer in file				|


## Helper routine and data

|addr	|data	|BM addr|BM data|					|
|-------|-------|-------|-------|-------------------|
|$0107	|$3F0C	|$2107	|$201C	|System Initialize	|
|$010A	|$3D7A	|$210A	|$2000	|get a char			|
|$010D	|$3C00	|$210D	|$200D	|put a char			|
|$0110	|$1500	|$2110	|$202F	|break check		|
|$0113	|$1600	|$2113	|$3600	|top of program		|
|$0116	|$3AFF	|$2116	|$6AAF	|RAM end			|
|$019D	|$07EC	|$219D	|$2060	|Tape read			|
|$01AB	|$0EE2	|$21AB	|$F000	|jump to monitor	|
|$01C1	|$07F8	|$21C1	|$2040	|Tape write			|
|$01E9	|$01	|$21E9	|$09	|pause key			|
|$01F0	|$03	|$21F0	|$0A	|break key			|
|$020B	|$3C	|$220B	|$3C	|number of columns	|
|$020F	|$08	|$220F	|$7F	|bask space			|
|$0213	|$18	|$2213	|$08	|Delete one line	|
|$080F	|$03	|$280F	|$03	|break key			|
|$1006	|$80	|$3006	|$80	|High address of INP|
|$14BE	|$80	|$34BE	|$80	|Low  address of OUT|

## コマンド

月刊ASCII 1980年1月号 p.40 「micro PASCAL 第一回文法の解説」、月刊ASCII 1980年12月号 p.154  「micro PASCAL 68より

- A		LOAD
- B		Bottom		ポインタを最後の行の次にもっていきinputモードにする
- Cn	Change Page	プログラムの領域の先頭アドレスを指定する。
- Dn	Delete		ポインタの指している行からn行削除する。ポインタは削除された次の行に移る。
- En	End Memory	使用メモリの終わりを指定する。値は16進数。	
- Fs	Find		ポインタの指している行から下へ、sで始まる行を探し、そこへポインタを移す。
- G/				プログラムを実行する(RUN)
- H		Monitor		BASICMASTERのモニタへ移る($F000)
- Is	Insert		ポインタの指している行の上にsを挿入する。ポインタの指示は変わらず。
- I					ポインタをそのままにしてInputモードにする。
- K/	Kill		インタプリタをイニシャライズする(NEW)
- Ln	Last		ポインタをn行上の行に移す。
- M		Memory		ファイル領域の残りバイト数を10進表示。
- Nn	Next		ポインタをn行下の行に移す。
- Pn	Print		ポインタの示す行から下へn行印刷する。ポインタは最後に印刷した行に移る。
- Rs	Retype		ポインタの指している行をsで置き換える。ポインタは、次の行に移る。
- S		SAVE
- Tn	Top			ポインタを最初の行にもっていきn行したの行に移す

## ASCII誌掲載のmicro PASCAL 68ダンプリストについて

判別しづらい、抜けている箇所

- 02FE 8D
- 03F2 CE
- 0779 00

## 参考文献

- 月刊ASCII 1980年1月号 p.40 「micro PASCAL 第一回文法の解説」
- 月刊ASCII 1980年2月号 p.144 「micro PASCAL」
- 月刊ASCII 1980年3月号 p.137 「micro PASCAL」
    - 文法説明、エディタの使い方
- 月刊ASCII 1980年5月号 p.119 「micro PASCAL-TRS」
- 月刊ASCII 1980年6月号 p.148 「micro PASCAL-MZ」
- 月刊ASCII 1980年12月号 p.154  「micro PASCAL 68」
- エンサイクロペディア・アスキー第5巻 p.368 「micro PASCAL 第一回」 
- エンサイクロペディア・アスキー第5巻 p.382 「micro PASCAL 第二回」
- エンサイクロペディア・アスキー第6巻 p.406 「micro PASCAL」
- エンサイクロペディア・アスキー第6巻 p.415 「micro PASCAL-TRS」
- エンサイクロペディア・アスキー第6巻 p.421 「micro PASCAL-MZ」
- エンサイクロペディア・アスキー第7巻 p.434 「micro PASCAL 68」
- BASIC MASTER USER'S GROUP NO.4 1982 APR p.12
    - SAVE(S),LOAD(A)コマンドの追加。$2040-$20F3にSAVE/LOADルーチン
- BASIC MASTER USER'S GROUP      1982 MAY,JUN p.3
    - プリンタ出力、1200bps SAVE/LOAD
