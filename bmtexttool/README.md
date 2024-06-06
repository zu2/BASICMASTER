# BASICMASTER text tools

## 何のためのツールか

UNICODEで書かれたテキストファイルをBASICMASTERに持ち込む、あるいはその逆を行うツール。

## txt2bm

UNICODEテキストファイルをBASICMASTER BASIC形式のテキストに変換する。

./bm2txt file

- 結果は標準出力なので適宜リダイレクトして保存する
- 改行はCRのみに変換
- グラフィックキャラクタ（絵文字）はbmchar.hに従って変換
- （変換詳細はbmchar.hを見た方が速い）
- 変換したファイルをbmtape2にてbinファイルに変換すると、bm2/ebmjrでカセットLOADできるようになる。

## txt2bm

BASICMASTER BASIC形式のテキストファイルを UNICODEのテキストに変換する。

./txt2bm file

BASIC形式のテキストは、例えばbmtape2などのツールでwavから吸い出して作成できる。

LISTコマンドの結果を拾えるといいのですが。

## game2bin

GAME言語のプログラムをGAME68/BMの内部形式に変換する。

キャラクタコードの変換機能はまだ無い。

変換したプログラムをmotorola S/Intel Hexにすることで、各種エミュレータに持ち込める。

## ntb2bin

BASICのプログラムをNAKAMOZU Tiny BASIC形式に変換する。

キャラクタコードの変換機能はまだ無い。

変換したプログラムをmotorola S/Intel Hexにすることで、各種エミュレータに持ち込める。

## txt2mot

テキストファイルをMotorola S形式に変換する。オフセット指定できる。

game2binやntb2binで変換したファイルをエミュレーターに持ち込む用途を想定している。

## bin2bas

bmtape2で変換したbinファイルから、BASICソースプログラムっぽい部分を抽出する。未完成。

そもそもbmtape2にもこの機能はあるのだけど、壊れたwavから読ませるとうまくいかないので、これを作った。

壊れたファイルを読むので文字化け上等であるが、それでも読みたい場合に。
