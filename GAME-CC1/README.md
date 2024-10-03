# GAME68 Cross Compiler

by ZUKERAN, shin  (@zu2, TEW SOFT)

This is a practice of a compiler for the GAME language. It will be converted to MC6800 assembly language.
I created it on MacOS, but it doesn't use any special routines so it should work on Linux too.

GAME言語のコンパイラの習作です。MC6800のアセンブリ言語に落とします。
MacOSで作成しましたが、特別なルーチンは使っていないのでLinuxでも動くはず。

作成期間2週間(2024/7/24-8/8)

- [GAME language](./about-GAME.md)

## コンパイル方法

```
.game.mot: $(OBJS) gamecc
		./gamecc $< |tee $*.asm
		a09 -oM00 -S$*.mot -L$*.lst $*.asm
```

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

## コンパイラの動作について

- https://www.sigbus.info/compilerbook を参考に作りました。ステップ14まで
- ASTから直接オブジェクトコードを生成します
- https://github.com/DoctorWkt/acwj/tree/master/64\_6809\_Target を参考にしました。
	- 現在レジスタに何が入っているのかを追跡し、余計なコードをなるべく生成しないようにしています
	- 配列アクセスで使った一時変数をなるべく使いまわせるように考慮しています
- optimize.c で簡単な最適化を行っています
- forとdo loopでは、制御変数による配列アクセスを高速化しています
	- 制御変数を書き換えるとおかしくなります
	- (ループ内での書き換えはチェックしていますが、サブルーチン先で書き換えたらおかしくなります）
- for loopでは比較に>0ではなく>=0を使うようにしています
	- forの終値を+1しているので、32767の場合の動作がインタプリタと非互換です
	- これをやらないとループ1回あたり10cycle遅くなります
- 通常の比較でも、定数との比較では+1をしています。32767との比較はおかしくなります。
- PSH/PULは遅いので、なるべく生成しないようにしています
- MUL/DIVのサブルーチンは、なるべく引数をAccABとIXで値渡しできるようにしています
- RND関数の生成方法を高速化していて、厳密には互換がありません

### 直したいけど力不足な部分

- 共通式の括りだしができていません
- 8bitで済む演算は8bitにしたいのですが、まだ型の概念がないのでできていません
- 一時変数を大量に使うようなコードをコンパイルするとコンパイラが落ちます。
	- インタプリタには1行72文字の制限があるから、たぶん大丈夫ですが、クロスコンパイルだと制限を超えるかもしれません


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
