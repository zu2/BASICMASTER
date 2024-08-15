# GAME68 Cross Compiler

by ZUKERAN, shin  (@zu2, TEW SOFT)

GAME言語のコンパイラの習作です。MC6800のアセンブリ言語に落とします。
MacOSで作成しましたが、特別なルーチンは使っていないのでLinuxでも動くはず。

作成期間2週間(2024/7/24-8/8)

## コンパイル方法

./gamecc ソースコード > アセンブラソース

## 実行方法

作成した mot や hex をエミュレータに持っていき、CALL $2000で実行してください。
インタプリタ不要です。オブジェクト単体で動きます。

## 非互換・バグ

たくさんあるはず。わかっているものを下に書きます。

- <del>"..." と / が空白なしで連続できない。これ以外にもあるはず。</del>
	- 姑息な修正をしましたが、問題あるかもしれません
- ? による数値入力は、インタプリタでは式が書けるけど、こちらは数値のみ。
- [=1 MUSIC% による音がインタプリタと一致しない。なんで?
- ソースの文字コードはUNICODEです。BM文字との対応はbmchar.cを見てください。

不親切な点

- エラーがあるといきなり落ちます。ソースコードのエラーも同様
- あると思うなエラーメッセージ
- アセンブルしないとわからないエラーもあります。<del>行番号の重複など。</del>

## サンプル

TinyGalaxyをGAME言語に書き直したものです。アセンブラルーチン無しでこの速度が出ています。
英数で加速、英記号・カナ記号が左右移動です。

- TinyGalaxy.game
- TinyGalaxy.mot コンパイル済み。CALL $2000で実行

## インタプリタとの速度比較動画

[https://www.youtube.com/watch?v=G-P93oGVb98](https://www.youtube.com/watch?v=G-P93oGVb98)
[https://www.youtube.com/watch?v=cs1DSGrmPYA](https://www.youtube.com/watch?v=cs1DSGrmPYA)

## 参考文献

- GAME68コンパイラ ASCII 1982年5月号 P.149 (エンサイクロペディア・アスキー第9巻 P.99)
- [GAME68コンパイラ : 電子工作やってみたよ](https://telmic.exblog.jp/30174191/)
- GAME ASCII 1978年7月号 P.66
- GAME ASCII 1978年8月号 P.42
- GAME ASCII 1978年9月号 P.68
- GAME ASCII 1978年10月号 P.66
