# BASICMASTER / L2 BASIC Compiler

Softwares and Documents for Hitach BASIC MASTER L2/L2II/Jr.

## L2 BASIC Compiler

L2 BASICで書いたL2 BASICのコンパイラ。コードはK-code(kumajiriの中間言語）に落ちます。

GAMEでGAMEのコンパイラが書けるんだからBASICでもできるでしょ、でも機械語に落とすのは難儀なのでK codeでお茶を濁しました。

- [COMP.S](COMP.S) コンパイラソースコード

BMUG 1982.Apr とは差異があります。

## 当時のマニュアルを人間OCRしたもの

### LEVEL2II BASIC COMPILER V0.5 (未完成) TEW SOFT

- コンパイルできる文
    - LET,PRINT,INPUT,GOTO,GOSUB,RETURN,IF,CLR,REM,END
- コンパイル可能なデータ
    - 整数型
- コンパイル可能な演算
    - 四則とカッコのみ
- その他
    - CHR$、HEXなし
    - "〜" 内では、$80以上のキャラクタは使えない

### 使用法

- K-CPU (KUMAJIRI)をLOAD。$0B00〜$0FFF
- POKE $74,$10:NEW
- LOAD COMP↩️

- 1-9999行にプログラムを書く。
- 最後は必ずENDをつける。エラーがなかったらコンパイル。RUN 10000↩️
- 実行は CALL $7000↩️
- 終了時はブレークがかかります。
- タマに暴走するけどカンニンね

- MB-6880L2はPOKE文を書き直す必要があります。
- 行番号1-4迄のテストでは、L2 BASIC 約15秒、コンパイラ 約8秒でした。

※K-CPUのスタックは$7FFEにしてください($0B07と8に入ってる）
※30152はGOTO 31050に修正 （githubのは修正済み）



