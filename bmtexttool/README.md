# BASICMASTER text tools

## 何のためのツールか

UNICODEで書かれたテキストファイルをBASICMASTERに持ち込む、あるいはその逆を行うツール。

## txt2bm

UNICODEテキストファイルをBASICMASTER BASIC形式のテキストに変換する。

./bm2txt file

- 結果は標準出力なので適宜リダイレクトして保存する
- 改行はCRのみに変換
- グラフィックキャラクタ（絵文字）はbmchar.hに従って変換
- （変換詳細は[bmchar.h](./bmchar.h)を見た方が速い）
- 変換したファイルをbmtape2にてbinファイルに変換すると、bm2/ebmjrでカセットLOADできるようになる。

## bm2txt

BASICMASTER BASIC形式のテキストファイルを UNICODEのテキストに変換する。

./bm2txt file

BASIC形式のテキストは、例えばbmtape2などのツールでwavから吸い出して作成できる。

LISTコマンドの結果を拾えるといいのですが。

## txt2bmf

BASICMASTER完全フルキーボード化プログラム用のテキストに変換する。
(I/O誌1980年3月号P.89、1980年8月号P.96参照）

これで変換したテキストファイルをエミュレータにPasteできる。

j68ではファイルから読み込むときも変換可能なのでこのプログラムは不要かも（Keyboard→FullChar Patch）


## game2bin

GAME言語のプログラムをGAME68/BMの内部形式に変換する。

BASICMASTER独自キャラクタコードは、事前にtxt2bmで変換しておく。

変換したプログラムをmotorola S/Intel Hexにすることで、各種エミュレータに持ち込める。

下記の例では、GAME-MBに合わせて先頭アドレスを $1A00 としている

```
txt2bm source.game | game2bin  - > source.bin
txt2mot source.bin 0x1A00 > source.mot
```

あるいは

```
txt2bm source.game | game2bin  - > source.bin
~/bin/objcopy -I binary -O srec --adjust-vma 0x1a00 source.bin source.mot
```

この後、GAME-MBが起動しているエミュレータに .mot を drag&amp;dropし、== でテキスト末尾位置を修正する。
```
> ==
> ??=&
> #=1
```

## ntb2bin

BASICのプログラムをNAKAMOZU Tiny BASIC形式に変換する。

BASICMASTER独自キャラクタコードは、事前にtxt2bmで変換しておく。

変換したプログラムをmotorola S/Intel Hexにすることで、各種エミュレータに持ち込める。

下記の例では、BASICMASTER版NTBに合わせて先頭アドレスを $3901 としている

```
txt2bm source.ntb | ntb2bin  - > source.bin
txt2mot source.bin 0x3901 > source.mot
```

あるいは

```
txt2bm source.ntb | ntb2bin  - > source.bin
~/bin/objcopy -I binary -O srec --adjust-vma 0x3901 source.bin source.mot
```

この後、NTBが起動しているエミュレータに .mot を drag&amp;dropし、OLDコマンドでテキスト末尾位置を修正する。


## txt2mot

テキストファイルをMotorola S形式に変換する。オフセット指定できる。

game2binやntb2binで変換したファイルをエミュレーターに持ち込む用途を想定している。

## bin2bas

bmtape2で変換したbinファイルから、BASICソースプログラムっぽい部分を抽出する。未完成。

そもそもbmtape2にもこの機能はあるのだけど、壊れたwavから読ませるとうまくいかないので、これを作った。

壊れたファイルを読むので文字化け上等であるが、それでも読みたい場合に。
