# BASICMASTER

日立ベーシックマスター L2/L2II/Jr 用のソフトウェアとドキュメント

Softwares and Documents for Hitach BASICMASTER L2/L2II/Jr.

blog: [https://www.zukeran.org/shin/d/category/pc/basicmaster/](https://www.zukeran.org/shin/d/category/pc/basicmaster/)

## LIFE2

- LIFE GAMEの亜種。雑誌か何かで読んだものだと思うが覚えていない
- 詳細は [life2/README.md](life2/README.md) 参照

## Tiny Galaxy

- ゲームGalaxyの簡易版
- 詳細は [TinyGalaxy/README.md](TinyGalaxy/README.md) 参照

## L2 BASIC Compiler

- 1982年にBASICで書いたBASICコンパイラの習作
- 制限ありまくりです
- 詳細は [L2BASCOM/README.md](L2BASCOM/README.md) 参照

## NTB

- NAKAMOZU Tiny BASIC port to BASICMASTER.
- 簡易グラフィック機能を追加してあります
- 詳細は [NTB/README.md](NTB/README.md) 参照

## GAME68

- GAME3言語の移植版。BMUGで配布されていたGAME-MBではありません。
- GAME-MBに合わせて機能追加しています。
- 変数・特殊変数を0ページではなく4ページに置いているので、GAME Compilerを作るのは大変かも
- 詳細は [GAME68/README.md](GAME68/README.md) 参照
- GAME言語については[GAME-CC1/about-GAME.md](GAME-CC1/about-GAME.md) 参照

## GAME-CC1 (GAME68 cross compiler)

- GAME68のクロスコンパイラ
- アセンブラソースを出力します。[a09](https://github.com/Arakula/A09)でアセンブルできます。
- 10-50倍程度に高速化されます。
- 詳細は [GAME-CC1/README.md](GAME-CC1/README.md) 参照
- 画面表示、キー入力などのサブルーチン([GAME-CC1/gamecc.asm](GAME-CC1/gamecc.asm))を変更すれば、MC6800/MC6802を使った他機種でも動くと思います。

## KUMAJIRI

- KUMAJIRI compiler
- 詳細は [KUMAJIRI/README.md](KUMAJIRI/README.md) 参照

## micro PASCAL 68

 ASCII誌に掲載されたmicro PASCAL 68を動かすためのパッチです

- 詳細は [microPASCAL68/README.md](microPASCAL68/README.md) 参照

## bmtexttool

- UNIX環境とBASIC MASTER内とのテキストデータの変換ツール
- 詳細は [bmtexttool/README.md](bmtexttool/README.md) 参照

## fcc for BASICMASTER

- Fuzix Compiler Kit でベーシックマスター用のプログラムを書くためのツール
- [zu2/fcc-for-basicmaster: Fuzix Compiler Kit for Hitach BASICMASTER](https://github.com/zu2/fcc-for-basicmaster)

## C言語について

クロスコンパイラを用いて開発できます。

- [EtchedPixels/Fuzix-Compiler-Kit: Fuzix C Compiler Project](https://github.com/EtchedPixels/Fuzix-Compiler-Kit)
- [zu2/CC6303: A C compiler for the 6800 series processors](https://github.com/zu2/CC6303)
- [zu2/chibicc-6800-v1: A Small C Compiler for MC6800 (fork from chibicc)](https://github.com/zu2/chibicc-6800-v1)
- [zu2/ack-6800: The Amsterdam Compiler Kit for MC6800](https://github.com/zu2/ack-6800)

kwhr0さんのページが参考になります。

- [6800エミュレータを書いたのでベーシックマスターJr.に載せてCC6303でクロス開発してみた](http://kwhr0.g2.xrea.com/bm2.html)
- [kwhr0/bm2-baremetal-demo](https://github.com/kwhr0/bm2-baremetal-demo)
- [kwhr0/bm2-xevious](https://github.com/kwhr0/bm2-xevious#)

## Documents

- 雑多なドキュメント
- [documents/README.md](documents/README.md)

## Tools

- 雑多なツール
- [tools/README.md](https://github.com/zu2/BASICMASTER/tree/main/tools)
