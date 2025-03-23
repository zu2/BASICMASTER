# KUMAJIRI compiler

TEW版KUMAJIRI/BMの改良版です。I/O別冊のものとは微妙に異なります。

公開に当たっては原著作者の津田伸秀様、工学社I/O編集部より許可をいただいております。
津田さんの意向により、ライセンスは MIT License とします。

- I/O別冊システムプログラムライブラリ2 p.224 「KUMAJIRIコンパイラ」

## DEMO

- [https://www.youtube.com/watch?v=cs1DSGrmPYA](https://www.youtube.com/watch?v=cs1DSGrmPYA)

## Directory List

- K-COMP.B.mot	KUMAJIRIコンパイラのバイナリ
- K-COMP.S		KUMAJIRIコンパイラのソースリスト
- K-COMP.S.mot	上記のmotorola S形式
- K-CPU.asm		K-CODEインタプリタソースリスト
- K-CPU.mot		K-CODEインタプリタのバイナリ
- README.md		このファイル
- K-ALL-ranpaku.mot,hex	RANPAKUエディタ入りの全ファイル

## recompile KUMAJIRI Compiler

K-CPU.mot, K-COMP.B.mot を読み込ませる。

K-COMP.S.mot を読み込ませると、$3400-$4798にソースコードが配置される。
コンパイラに認識させるため、($1FFC,D)に$3400、($1FFE,F)に$4798を書く。
コンパイル終了後に暴走しないように$1FA0,$1FA5にSWI($3F)を書く。
$2000番地から実行するとコンパイルを行う。

## Memory Map

TEW版

- 00E0-00F3	K-CPU work area
- 0700-07FF K-Compiler Work (Variables)
- 0B00-0FB5	K-CPU
- 1000-		K-Compiler Work	(T)
- 1500-		K-Compiler Work	(S)
- 1600-		K-Compiler Work	(I)
- 1800-		Hitachi EDITOR
- 1FA0		return from compiler to hitach editor (no error).
- 1FA5		return from compiler to hitach editor (with error).
- (1FFC,D)	Source program Top address-1
- (1FFE,F)	Source program End address+4
- 2000-32FF	K-Compiler
- (324F,50)	jump to Compile end (no error)
- (32E5,E6)	jump to Compile end (error)
- 3400-		Source code

本ディレクトリにはEDITORは含まれていません（日立のアセンブラを流用したため）。

## KUMAJIRI Compiler

コンパイラは($1FFC,D)+1からソースプログラムがあると想定している。
また、($1FFE,F)-4がプログラム末と仮定し、そこに終了マーク（$FF)を上書きする。
これは日立エディタがプログラム末にEOF($0D)を置いているため。

変更する場合は、$201Eの$1F,$FE、$2036の$1F,$FCを修正する。
-4の値は$202D($FF,$FC)で修正できるが、$FFによる上書きの修正は大変。

+1の修正もプログラムの大幅変更が必要。ソースプログラム開始位置をポイントするエディタの場合は、ソーストップに空行を入れておけば良い。

コンパイル終了後は$1FA0、エラー発生時は$1FA5に飛ぶ。
エディタが$1800-に無い場合は、適宜書き換えるか、$1FA0,$1FA5をSWI($3F)にしておく。
これらは、$324F($1F,$A0)、$32E5($1F,$A5)で変更できる。

## Hitachi Editor版

I/O別冊に投稿した記事では、エディタは日立エディタ・アセンブラを流用した。
このディレクトリには含まれていない。

エディタからはG2000でコンパイルすることを想定している。

工学社I/O 1982年4月号p.304、ベーシックマスター活用研究 L3,Jr p.95の日立エディタ・アセンブラは$2F00-$3EFFを使うため、リロケートが必要
（ソース開始アドレスも$3EFC,Dになっている）

エディタのソースコードの行末はCR($0D)、ファイル末は$0Dの後に"EOF",0D,$00。
KUMAJIRIのソースコードの行末はCR($0D)、ファイル末は$0Dの後に$FF。
そのため、コンパイラ先頭でソース終了アドレス-4に$FFを書き込んでいる。

メモリが32KBを超える場合に、SOURCE TOP ADDRから先に進まなくなる。オリジナルにも存在するBUG.
($18B7のBGTはBHIが正しい。2E→22にする。オリジナル日立アセンブラなら$2FB7の$2E→$22）

- [https://youtu.be/cs1DSGrmPYA?si=2iNDXDNd7sduzvI1](https://youtu.be/cs1DSGrmPYA?si=2iNDXDNd7sduzvI1)

## RANPAKU Editor版

KUMAJIRIコンパイラ、中間言語インタプリタ、エディタ込みの一式セットです。

エディタは月刊I/O 1980年8月掲載のエディタをBASICMASTERに移植したものです。

I/O 1981年4月号の移植とは別です。カセットSAVE/LOADも追加してあります。

hexまたはmot読み込み後、CALL $1800でエディタが起動します。

- [K-ALL.ranpaku.hex](./K-ALL.ranpaku.hex)
- [K-ALL.ranpaku.mot](K-ALL.ranpaku.mot)

コンパイラソースの変更箇所は以下の通り。

```
--- KUMAJIRI-source.S	2024-05-25 12:28:47
+++ KUMAJIRI-ranpaku.S	2024-07-03 17:25:30
@@ -1,7 +1,7 @@
 *** KUMAJIRI COMPILER ***
 //" - TRANSFORM K-CODE -"//
-A=$1FFE A=A(0) A:-4)=$FF
-X=$1FFC X=X(0)
+A=$180B A=A(0)
+X=$1807 X=X(0)
 " SOURCE PROGRAM FROM $" ??=X+1
 /" OBJECT PROGRAM FROM " Z=? Y=Z+3
  " VARIABLE ADDRESS FROM " V=?
@@ -168,10 +168,10 @@
 &
 //" TRANSFORMATION END."
 /"   OBJECT PROGRAM : $" ??=Z "-$" ??=Y+1 Y(1)=$776 Y(2)=$A033
->=$1FA0
+>=$1802
 IL. /" ILLEGAL NESTING " EFLAG=1 ]
 LORN. ;=B>"9"&(B<"A").(B<"0").(B>"Z") B=0 : B=1 ; ]
 ERROR. A=X @ A=A-1 @=A:0)=$D B=$A3 B(0)=N
-@ A=A+1 $=A:0) @=A=X / >=$1FA5
+@ A=A+1 $=A:0) @=A=X / >=$1802
 *
 ****
```

### 使用例

- [https://youtu.be/ylioAQ-ejrg?si=YXtFhy-8xBeEI8f7](https://youtu.be/ylioAQ-ejrg?si=YXtFhy-8xBeEI8f7)

### TEW版+RANPAKUエディタ版メモリマップ

- 00E0-00F3	K-CPU work area
- 0700-07FF K-Compiler Work (Variables)
- 0B00-0FB5	K-CPU
- 1000-		K-Compiler Work	(T)
- 1500-		K-Compiler Work	(S)
- 1600-		K-Compiler Work	(I)
- 1800-		RANPAKU EDITOR
- 1802		return from compiler to ranpaku editor.
- (1807,8)	Source program Top address-1
- (180B,C)	Source program End address
- 2000-32FF	K-Compiler with RANPAKU
- (3246,47)	jump to Compile end (no error)
- (32DC,DD)	jump to Compile end (error)
- 3400-		Source code



## 掲載雑誌・書籍

- 月刊I/O 1980年5月号 p.103 「6809シミュレーター」津田伸秀
    - ここで使われているインタープリタがK-CPU、K-BUGがKUMAJIRIで作成されたモニタプログラム
- 月刊I/O 1980年8月号 p.103 「6800用構造化言語 KUMAJIRIコンパイラ」津田伸秀
	- インタープリタは$0B00-$0FFF、コンパイラワークが$1000-$17FF、コンパイラは$2000-$2FFF
    - ソースプログラム開始エリアは($707,8)、終了は($709,A)
- 月刊I/O 1981年4月号 p.115 「KUMAJIRI/BM」C.CATO
	- C.CATO版
	- エディタはオリジナルを移植($700-$AFF)。ワーク領域は$F2-$F9
- 月刊I/O 1981年6月号 P.173 「de BUG」 C.CATO
	- Fコマンドはプログラムが壊れるとのこと
- I/O別冊システムプログラムライブラリ2 p.224 「KUMAJIRIコンパイラ」
	- いわゆるTEW版KUMAJIRI/BM
	- エディタは日立アセンブラ32KB版をそのまま使う($6F00-)
	- ソースプログラム開始エリアは($6EFC,D)、終了は($6EFE,F)
- BMUG会報 1983.11 p.12によると、この他にもBMUG会員による移植が2つ（以上？)存在したようだ

