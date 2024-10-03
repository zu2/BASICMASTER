# GAME68 on BASIC MASTER 

GAME68 interpriter for Hitach BASIC MASTER.

- [GAME language](https://github.com/zu2/BASICMASTER/blob/main/GAME-CC1/about-GAME.md)

## original source code

月刊アスキーの1978年7月号から10月号（エンサイクロペディア・アスキー Vol.2,3）

## 参考にしたソースコード

- https://flexonsbd.blogspot.com/2024/03/6800game6809.html

## DEMO


- YouTube: https://youtu.be/H7nNazKI7cg?si=s2Mx-cjKBpMB6fUf
![https://youtu.be/H7nNazKI7cg?si=s2Mx-cjKBpMB6fUf](./20240604-GAME68-2.png)


## 追加機能

SAVE/LOAD/APPENDの仕様は変更するかも。
```
[S FILENAME		SAVE
[L FILENAME		LOAD
[A FILENAME		APPEND
```
以下、GAME-MBに合わせた

論理演算
```
A\&B				AND
A.B				OR
A!B				XOR
```

特殊変数
```
\			TIME1,2	（1秒タイマーの値）
^			CURSOR位置($0F,$10)
^$			CURSOR位置のアドレス(JSR $FFF5)
[			現在のキーボード状態(JSR $F00F)、キー入力がなければ0を返す
[R			delay無しのキースキャン（未実装）。$F00Fは待ちがあって遅い
```

モニタ、音楽

GAME-MBは [=0 で専用のモニタ（GAME-MB MONITOR)に移動し、SAVE/LOAD/Verifyが行えるが、本移植では標準モニタに移動するだけである。

GAME-MBのモニタは、S/L/Vの他、Tranfer/Printが行える（#もPに同じ）。

MUSICはGAME-MBでは最後に % が必要。本移植ではあってもなくても良い。

```
[=0			標準モニタに移動
[=1	STR		MUSIC
```

## オリジナルと異なる部分

数値入力（A=?など）の時にプロンプトを出さないようにした。オリジナルは ':' が表示されていた。

## BUG

- bm2で起動時にゴミが表示される（修正済み。初期化漏れがあった）
- テープSAVE中にBREAKすると、動作がおかしくなる

## 資料

- [エンサイクロペディア・アスキー volume 2 - 国立国会図書館デジタルコレクション](https://dl.ndl.go.jp/pid/10259290)
- [エンサイクロペディア・アスキー volume 3 - 国立国会図書館デジタルコレクション](https://dl.ndl.go.jp/pid/12631628)
- [エンサイクロペディア・アスキー volume 4 - 国立国会図書館デジタルコレクション](https://dl.ndl.go.jp/pid/12631630)
- [エンサイクロペディア・アスキー volume 5 - 国立国会図書館デジタルコレクション](https://dl.ndl.go.jp/pid/12631635)
- [エンサイクロペディア・アスキー volume 6 - 国立国会図書館デジタルコレクション](https://dl.ndl.go.jp/pid/12631585)
- [6809 / 6800とFLEX: 6800用GAMEインタプリタとコンパイラの6809への移植がようやく完成](https://flexonsbd.blogspot.com/2024/03/6800game6809.html)
- [GAME68コンパイラ : 電子工作やってみたよ](https://telmic.exblog.jp/30174191/)
- [GAME68 - ずmemo](https://wiliki.zukeran.org/old/1543448974.html)
