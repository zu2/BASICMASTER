# BASICMASTER Jr用ハイレゾグラフィックルーチン

1982年ごろにMB-6881改造版用に作成したルーチンをJr化したものです。

文字表示のためには、$1800-$1FFFの2KBキャラクタROMと同じデータが必要です。

PRINT文は、独自のフックを持っているため、このルーチンでは文字表示できません。

## デモ

<iframe width="560" height="315" src="https://www.youtube.com/embed/sV_8Xb4gq7k?si=Nsj4-E10YTjw7wS7" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>

## 初期化・文字列表示

$1003は機械語から呼ぶためのルーチンです。

- CALL $1000
	-- グラフィック表示へ切替、文字入出力をフック
- JSR $1003
	-- CR終端の文字列を表示	機械語からXに文字列先頭を渡して呼ぶ
- CALL $1006
	-- 通常の文字入出力・テキスト表示へ戻す

## GET/PUT

他機種のGET@,PUT@のようなルーチンです。

- CALL $1200,A$
	-- 現在のカーソル位置の8×8ドットを文字列へ保存
	-- あらかじめ8文字分の領域を持つ文字列を渡す
	-- 保存データは下段から上段の順
- CALL $1203,A$
	-- 文字列の8バイトを現在のカーソル位置へ描画

## グラフィックルーチン

Fは0,1,255のいずれかの式。0は消去、1は点灯、255は反転。

四角形・継続線を書く$1406の最後のパラメータは変数名を見ている。値ではない。

- CALL $1400,X,Y,P
	-- 点の描画・消去・反転	
- CALL $1403,X,Y,P
	-- 点の状態を変数へ格納。Pは数値変数
- CALL $1406,X1,Y1,X2,Y2,F
	-- 直線	
- CALL $1406,X1,Y1,X2,Y2,F,B
	-- 四角形の枠（最後のBは変数名を見ている）
- CALL $1406,X1,Y1,X2,Y2,F,F
	-- 塗った四角形(最後のFは変数名を見ている）
- CALL $1409,X,Y,R,F
	-- 円周	
- CALL $140C,X,Y,F
	-- PAINT
- CALL $140F,X2,Y2,F
	-- 現在位置から終点までの直線
- CALL $140F,X2,Y2,F,B
	-- 現在位置から終点までの四角形の枠(最後のBは変数名を見ている）
- CALL $140F,X2,Y2,F,F
	-- 現在位置から終点までの塗った四角形(最後のFは変数名を見ている）
- CALL $1412,V
	-- 画面全体を指定バイトで埋める	
- CALL $1415
	-- 画面全体のビット反転
