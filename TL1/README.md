# TL/1 - Tiny Language 1

# ASCII掲載のダンプリストについて

- Motorola S形式
- 分割アセンブル・RMBによる空き部分の違いでチェックサムが変わるかも
	- ダンプリストと、ソースリストを比べて確認する必要あり

# BMUG移植版について

## TL/1本体

- BMUG会報 NO.2 FEB 1982 P.3-8
	- エディタとしてGAMEを利用する
- $10から利用しているワーク領域を$80以降に移動
- $13E1 $05 →0a	文字コードの相違
- $1925 $05 →0a	同上
- $20dd	$5e →7f	同上

## 機能追加 & BUG FIX

- BMUG会報 NO.2 FEB 1982 P.9-14
	- CALL,CURSOR,TAB,PLOT
- BMUG会報 NO.3 MAR 1982 P.2 TEW SOFT
	- SAVE/LOADルーチンの追加(GAME側を変更）
	- 下記の変更後、GAMEから >=$E00
	- L2
		- $0F2B 39
	- L2II
		- $0EB6 $FF $E9
		- $0EBC $FF $F2
		- $0EBF $BC $7C
		- $0EC2 $FF $F2
		- $0FCE $FF $F2
		- $0F2B $39
		- $0F59 $FF $E6
- BMUG会報 NO.5 MAY 1982 P.4 TEW SOFT
	- 動作が安定する
	- $0F20 $47 $53
- BMUG会報 NO.6 JUN 1982 P.3
	- MHIGH,MODが動作しなかった。変数アドレス変更漏れ
	- $1A1E $16 $86
	- $1618 $16 $86
	- $1A24 $17 $87
	- $161C $17 $87
- BMUG会報 NO.21 SEP 1983 P.3-7
	- スクリーンエディタ・Jr対応グラフィックルーチン追加
- BMUG会報 MAY,JUN 1984 P.31-35
	- TL/1によるグラフィック

# 掲載誌

- ASCII 1980年6月号 P.82-90  「新言語 Tiny Language 1」
	- 「TL/1言語の概要」
	- エンサイクロペディア・アスキー Vol.6 P.352
- ASCII 1980年7月号 P.144-150 「新言語 Tiny Language 1」
	- 「ライブラリの登録」
	- エンサイクロペディア・アスキー Vol.6 P.361
- ASCII 1980年8月号 P.148-155 「新言語 Tiny Language 1」
	- 「コンパイラの作り方」
	- 「汎用I/Oルーチン」
	- エンサイクロペディア・アスキー Vol.6 P.368

# リンク

- [TL/1年表 | シロピョンの本棚](https://ameblo.jp/siropyon/entry-11477734742.html)
