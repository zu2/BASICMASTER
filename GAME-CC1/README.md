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

- "..." と / が空白なしで連続できない。これ以外にもあるはず。
- ? による数値入力は、インタプリタでは式が書けるけど、こちらは数値のみ。
- [=1 MUSIC% による音がインタプリタと一致しない。なんで?
- ソースの文字コードはUNICODEです。BM文字との対応はbmchar.cを見てください。

不親切な点

- エラーがあるといきなり落ちます。ソースコードのエラーも同様
- あると思うなエラーメッセージ
- アセンブルしないとわからないエラーもあります。行番号の重複など。
