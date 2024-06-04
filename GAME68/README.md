# GAME68 on BASIC MASTER 

GAME68 interpriter for Hitach BASIC MASTER.

## original source code

月刊アスキーの1978年7月号から10月号（エンサイクロペディア・アスキー Vol.2,3）

## 参考にしたソースコード

- https://flexonsbd.blogspot.com/2024/03/6800game6809.html

## DEMO


## 追加機能

SAVE/LOAD/APPENDの仕様は変更するかも。
```
[S FILENAME		SAVE
[L FILENAME		LOAD
[A FILENAME		APPEND
```
以下、GAME/BMに合わせた

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
[R			キースキャン？（未実装）
```

モニタ、音楽

```
[=0			モニタに移動
[=1	STR		MUSIC
```

## BUG

- bm2で起動時にゴミが表示される（修正済み。初期化漏れがあった）
