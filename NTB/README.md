# NAKAMOZU Tiny BASIC on BASIC MASTER 

NAKAMOZU Tiny BASIC port to BASIC MASTER L2.

## original source code

- [Nakamozu Tiny Basic /ASCII](https://hyamasynth.web.fc2.com/ACII_NTB/ACII_NTB.html)

## DEMO

[![NAKAMOZU Tiny BASIC on Hitachi BASIC MASTER Emulator](http://img.youtube.com/vi/M3DNZJXfutU/0.jpg)](https://www.youtube.com/watch?v=M3DNZJXfutU)
[![NAKAMOZU Tiny BASIC on Hitachi BASIC MASTER Emulator](http://img.youtube.com/vi/3svcPZtAtDg/0.jpg)](https://www.youtube.com/watch?v=3svcPZtAtDg)



## 追加機能

OLDはGAME言語の == と同じく、プログラム末尾を探してポインタを正しく設定する。

```
LOAD	FILENAME
SAVE	FILENAME
OLD
PLOT(X,Y,d)
	X = 0-63, Y=0-47, d=0,1,-1
```

## BUG?

行削除でダンマリになることがある。どこか壊してるっぽい。

<del>NTB連載最終回（ ASCII誌1979年7月号、エンサイクロペディア・アスキーVol.4）のπを求めるプログラムが正しく動かない。</del>PIのプログラムの入力ミスでした。

代入文のあとにゴミがあると無限ループ。例: A=1A
PICKUPでは ' ' を読み飛ばした後に ':' か 00 があれば、Z=1
その他は Z=0 になる。IX は増えないので、無限ループになる。
本来はエラーなので、Z=0 ならERR9か何かにすべき(本移植ではERR18）

```
Original Source
Line	address
0350	02D2		JSR	LET
0351	02D5		BRA	RN5
  :      :           :
0356	02E0	RN5	JSR	PICKUP
0357	02E3		BNE	RN5
```

初期化時にFLOD/FAUTの両方を2バイトクリアしてるが、後者はLPCTではないか?

```
0271	024E		LDX	#0
0272	0251		STX	FLOD
0273	0253		STX	FAUT
0274	0255		STX	WKUSE
```
