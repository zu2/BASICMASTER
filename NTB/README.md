# NAKAMOZU Tiny BASIC on BASIC MASTER 

NAKAMOZU Tiny BASIC port to BASIC MASTER L2.

## original source code

- [Nakamozu Tiny Basic /ASCII](https://hyamasynth.web.fc2.com/ACII_NTB/ACII_NTB.html)

## DEMO

<iframe width="560" height="315" src="https://www.youtube.com/embed/M3DNZJXfutU?si=HH_DTkVdCIpm8iQp" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>

## 追加機能

```
LOAD	FILENAME
SAVE	FILENAME
PLOT(X,Y,d)
	X = 0-63, Y=0-47, d=0,1,-1
```

## BUG?

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
