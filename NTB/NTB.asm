************************
*
* NAKAMOZU TINY BASIC
* FOR 
* VER.6.0
*
* ASSEMBLED
* BY H. YAMASHITA
* 1978,10
*
* port to BASICMASTER
* by ZUKERAN, shin
* (aka TEW SOFT, @zu2)
* 2024/05/31
*
************************
*
* OPERATING SYSTEM
*
;INI4	EQU	$FFEF		; I/O
;INEEE	EQU	$0028
;OUTEEE	EQU	$002B
;BREAK	EQU	$369B
; cursor position X,Y (0..32,0..23)
CURX    EQU     $000F
CURY    EQU     $0010
ASCIN	EQU	$0028
ASCOUT	EQU	$002B
BYTIN	EQU	$002E
BYTOUT	EQU	$0031
MONITOR	EQU	$F000
KBIN	EQU	$F00F
NMISET	EQU	$FFE6
NMICLR	EQU	$FFE9
CLRTV	EQU	$FFEF
CURPOS	EQU	$FFF5
BREAKV	EQU	$0004
WIDTH	EQU	32
HEIGHT	EQU	24
GWIDTH	EQU	WIDTH*2
GHEIGHT	EQU	HEIGHT*2
*
;NEG	EQU	$3C06		; GRAPHIC SUB
;PICK	EQU	$3BF7
;SET	EQU	$3BD7
;RESET	EQU	$3BE8
;REVERS	EQU	$3BF0
;VRAMPR	EQU	$3CB6		; OPTION(画面プリントルーチン)
*
DISP	EQU	$A894		; I/OのDISPLAYポインタ
*
************************
*
* SYSTEM ADRS
*
STACK	EQU	$8FF0		; スタック (RAMEND)
IXSTCK	EQU	$AFFF
VRAM	EQU	$0100		; VRAM ADRS
VRAMEND	EQU	$03FF
PIACRB	EQU	$E906		; H68 KEYSCAN用
*
************************
*
* BASIC WORK
*
	ORG	$74
*
WKA	RMB	2		; 一般WORK AREA
WKB	RMB	2
WKC	RMB	2
XS	RMB	2
SPS	RMB	2
MDS	RMB	2		; MOD SAVE
FS	RMB	1		; SAVE FLAG
ADP	RMB	4		; + ADRS設定WORK
RNDS	RMB	2		; 乱数レジスタ
FLOD	RMB	2		; LOAD FLAG
FAUT	RMB	2		; AUTO FLAG
LPCT	RMB	1		; LOOP COUNTER
CHCT	RMB	1		; 出力文字数レジスタ
WKUSE	RMB	2		; USING FLAG + WORK
*
	RMB	2
*
LNB	RMB	2		; 行番号レジスタ
EOB	RMB	2		; 入力バッファ末ADRS
EADRS	RMB	2		; TEXT用PROGRAM COUNTER
XSP	RMB	2		; IX用 STACK POINTER
CSP	RMB	2		; 算術 〃
FSP	RMB	2		; FOR NEXT用〃
DP	RMB	2		; READ,DATA POINTER
isBREAK	RMB	1		; BREAK key pressed?
	ORG	$0400
VTOP	RMB	(26+1)*2	; 変数エリア dummy,'A'〜'Z'
BOP	RMB	2		; システム変数 '['
EOP	RMB	2		; '\'
MEMEND	RMB	2		; ']'
IXV	RMB	20		; IX変数エリア '%0'〜'%9'
LSWK	RMB	2		; LOAD/SAVE IX save area
LSWK2	RMB	2		; LOAD/SAVE IX save area2
LSWK3	RMB	2		; LOAD/SAVE IX save area3
	ORG	$0500
DIRECT	RMB	2		; DIRECT MODE用行番号
BFFR	RMB	256		; 入力バッファ
BFFREND	EQU	*+1
EOBF	EQU	*		; 算術STACK
	RMB	16
CSTCK	EQU	*		; FOR NEXT STACK
	RMB	56
FSTCK	EQU	*
	ORG	$0700
IOBUF	RMB	256		; SAVE/LOAD BUF
*
BGNTXT	EQU	$3900		; TEXT開始ADRS DATA
ENDTXT	EQU	$4FFF		; TEXT終了〃   DATA
*
************************
*
	ORG	$B000
*
WSTRT	JMP	WSTRT2		; WARM START
START	JMP	CSTART		; COLD START
WSTRT2	JMP	END
INI4	RTS			; DUMMY
BREAK2	TPA
	STAA	isBREAK
	RTI
BREAK	TST	isBREAK
	RTS
* 
INEEE	TST	FLOD
	BEQ	INEEE1
	JMP	LHOOK
INEEE1	LDA A	#'_'
	JMP	ASCIN
OUTEEE	TST	FS
	BEQ	OUTEE1
	JSR	SHOOK
OUTEE1	JMP	ASCOUT
*
LINEIN	LDA A	FLOD		; 入力装置切り換え
;	BNE	CSTIN
	BNE	LN4
	LDA A	FLOD+1
	BEQ	LN2
	LDX	#LNB		; 自動行番号処理
	CLR B
	LDA A	#10
	JSR	ADBA
	JSR	PNUM
	JSR	OUTS
	BRA	LN6
*
LN1	BSR	CR1
LN2	LDA A	#'>'		; プロンプト出力
	JSR	OUT
	FCB	$8C		; （2バイトスキップ）
LN3	BSR	CR2
LN4	LDX	#BFFR
;LN4	LDX	#BFFR+1
;LN5	DEX
;	CPX	#BFFR-1
;	BEQ	LN3
LN6	EOR A	RNDS		; 乱数初期値変化
	STA A	RNDS
	BSR	INEEE
	STA A	0,X
	CMP A	#$7F		; 後退
	BEQ	BSDEL
	CMP A	#$08		; BS
	BEQ	BSDEL
	CMP A	#$0D		; CR
	BEQ	LNCR
	CMP A	#$20		; 他のコントロールコードは無視		
	BCS	LN6
LN61	BSR	OUTEE1		; 通常文字はエコーバック
	INX
	CPX	#EOBF
DC	BNE	LN6
ERR1	LDA B	#1		; 入力が長すぎる Error.1
	JMP	ERR
BSDEL	CPX	#BFFR		; 行頭？
	BNE	BSDEL1
	LDA A	#$07		; 行頭ならBell鳴らす
	BSR	OUTEE1
	BRA	LN6
BSDEL1	LDA A	#$7F		; それ以外は後退
	BSR	OUTEE1
	DEX
	BRA	LN6
LNCR	BSR	OUTEE1		; CR
	CLR	0,X		; CR処理
	TPA
	STA A	1,X		; 行番号相当部分をマイナスにしておく
	STX	EOB
	BRA	CR2
*
;LN7	;CMP A	#$18		; 一行まっ消?
;	;BEQ	LN1
;	CMP A	#$1F
;	BCS	LN6
;	INX
;	CPX	#EOBF
;DC	BNE	LN6
;ERR1	LDA B	#1
;	JMP	ERR
*
IN9	BRA	OUTEE1		; echo back
;	BSR	INEEE
;	CMP A	#3		; CNT-C?
;	BEQ	END
;	RTS
*
;CSTIN	BSR	IN
;	CMP A	#2
;	BNE	CSTIN
;	BRA	LN4
*
CR1	JSR	CR
CR2	LDX	#BFFR
	CLR	CHCT
RTNN0	RTS
*
PTST	LDX	BOP
	CLR	0,X
	INX
DEL2	STX	EOP
	LDA A	#$80		; TEXT終わりのマーク
	STA A	0,X
	RTS
*
;BRK	RTS			; DUMMY
BRK	PSH	A		; BREAK ?
	JSR	BREAK
	PULA
	BEQ	BRK1
	CLR	isBREAK
	LDS	#STACK
;	BSR	IN
	CLR	FLOD
	CLR	FS
	LDX	#BREAKM
	JSR	MGOUT
	JMP	ERRBRK
BRK1	RTS
*
LOAD	STX	LSWK
	BSR	PTST
	LDX	LSWK
APPEND	;BSR	CNT11
	TPA
	STA A	FLOD
	BRA	LOAD0
LOAD1	INX
LOAD0	LDA	0,X
	CMPA	#' '
	BEQ	LOAD1
	LDA B	#6
	STA B	FLOD		    ; LOAD Flag 0 or not 0
	CLR	LSWK2		    ; Filename -> $0043
	LDA A	#$43
	STA A	LSWK2+1
LOAD2	LDA A	0,X
	BEQ	LOAD3
	CMP A	#' '
	BEQ	LOAD3
	CMP A	#':'
	BEQ	LOAD3
	INX
	STX	LSWK
	LDX	LSWK2
	STA A	0,X
	INX
	STX	LSWK2
	LDX	LSWK
	DEC B
	BNE	LOAD2
	BRA	LOAD4
LOAD3	STX	LSWK
	LDX	LSWK2
LOAD31	LDA A	#' '
	STA A	0,X
	INX
	DEC B
	BNE	LOAD31
LOAD4	LDA A	#'.'
	STA A	$49
	LDA A	#'S'
	STA A	$4A
	CLR	$004C		    ; BLOCK No.
	BSR	LHSUB
	LDX	LSWK
	JMP	EDIT
*
LHOOK	STX	LSWK
	PSH B
LHOOK1	LDX	$003B
	CPX	#IOBUF+256
	BMI	LHOOK2
	INC	$004C		; inc BLOCK No.
	BSR	LHSUB
	BRA	LHOOK1
LHOOK2	;CPX	$003D
	;BPL	LHOOK4
	LDA A	0,X
	INX
	STX	$003B
	TST A
	BNE	LHOOK3
LHOOK4	LDA A	#$0D
	CLR	FLOD
LHOOK3	PUL B
	LDX	LSWK
	RTS
*				; LOAD a BLOCK
LHSUB	STX	LSWK3
	PSH A
	LDX	#IOBUF
	STX	$003B
	LDX	#IOBUF+256
	STX	$003D
	JSR	BYTIN
	LDA A	#'-'
	JSR	ASCOUT
	LDA A	#' '
	JSR	ASCOUT
	LDA A	$004C
	JSR	OUTHEX
	LDA A	#$0D
	JSR	ASCOUT
	PUL A
	LDX	LSWK3
	RTS
*
;CNT11	LDA A	#$11
COT	JMP	OUT
*
AUTO	TPA			; 自動行番号
	STA A	FAUT
	JSR	CLCPL
	SUB A	#10
	SBC B	#0
SETLN	STA A	LNB+1
	STA B	LNB
	RTS
*
STOP	JSR	PRINT		; STOP処理
	JSR	PSHX
	BRA	END1
*
CSTART	LDS	#STACK
	JSR	INI4		; COLD START INITILIZE
	CLR	isBREAK
	LDX	#STARTM
	JSR	MGOUT
	LDX	#BGNTXT
	STX	BOP
	LDX	#ENDTXT
	STX	MEMEND
*
NEW	JSR	PTST		; "NEW"
END	LDS	#STACK		; "END"
	CLR	isBREAK
	LDX	#BREAK2
	STX	BREAKV
	JSR	INITP
END1	LDX	#READY		; READY
	JSR	MGOUT
	JSR	CR
	TPA			;
	STA A	DIRECT		; ダイレクトモード処理
	LDX	#DIRECT
	STX	EADRS
*
EDIT	LDS	#STACK		; コマンド&エディットモードメインルーチン
ED1	JSR	LINEIN
	JSR	PKUP
	BEQ	ED1
	JSR	ATOB		; ASCII→BINART
	BCC	DRCT1		; 行番号なしならDIRECT MODE
	BSR	SETLN		; 行番号SET
	BSR	ED2		; エディターへ
	BRA	EDIT
*				;（エディター）
ED2	STX	XS
	STS	SPS
	JSR	SCHLIN		; 行番号のADRSをさがす
	STX	WKC
	BCS	INSERT		; if 同じ行番号がない
	JSR	NXTL1		; 同じ行番号がある
	INX			;   その行消去して
	SEI			;	後のTEXTを前へ
	JSR	NMISET
	TXS			;	つめる
	LDX	WKC
*
	DEX
	BRA	ED4
*
ED3	INX			; 転送
	PUL B
	STA B	0,X
	BNE	ED3
ED4	INX
	PUL B
	STA B	0,X
	CMP B	#$80
	BNE	ED3
*
	STX	EOP
INSERT	LDX	XS
	LDA A	0,X		; 挿入するバイト数計算
	BEQ	INST4
	LDA A	EOB+1
	SUB A	XS+1
	ADD A	#3
	LDX	EOP
	ADD A	EOP+1
	STA A	EOP+1
	BCC	INST0
 	INC	EOP
INST0	JSR	SIZE		; メモリ容量チェック
	BCC	INST1		; if OK
	STX	EOP		; if NG
	BRA	ERR2
*
DRCT1	BRA	DIRCT
*
INST1	LDS	EOP		; 計算した新しい最終ポインタ
				; IXは古いポインタ
	INX
INST2	DEX			; 挿入する分のスペースを開ける
	LDA A	 0,X
	PSH A
	CPX	WKC
	BNE	INST2
	LDS	LNB		; 行番号(BINARY)転送
	STS	0,X
	INX
	LDS	XS		; TEXTを入力バッファから
	DES			; 　TEXTエリアへ転送
INST3	INX
	PUL A
	STA A	0,X
	BNE	INST3
INST4	LDS	SPS
	JSR	NMICLR
	CLI
	RTS
*
INITP	LDX	#0		; FLAG & POINTER INITIALIZE
	STX	FLOD		; 2バイトずつCLR
	STX	FAUT
	STX	WKUSE
	CLR	FS
 	LDX	#FSTCK
	STX	FSP
	LDX	#IXSTCK
	STX	XSP
 	LDX	#CSTCK
	STX	CSP
 	LDX	#BGNTXT
	STX	DP
	RTS
*
* DIRECT MODE			;（ダイレクトモード）
*
DIRCT	JSR	CHVAR
	BCC	RN3
	LDX	#COMAND
	JMP	RN4
*
NXTL1	INX			; 次の行を探す
NXTL2	INX
REM	TST	0,X
	BNE	NXTL2
	RTS
*				;（行番号捜す）
SCHEND	TST	DIRECT
	BMI	SL1		; ダイレクトモードなら前(BOP+1)から探す
	LDX	EADRS
	JSR	CPBA		; AccAB>IX(0,1)?
	BCC	SL1		; 先の行番号ならEADRSから探す
SCHLIN	LDX	BOP
	INX
SL1	LDA A	LNB+1		; 行番号が見つから
	LDA B	LNB		; ないならERR
	BMI	ERRS
	BNE	SL3
	TST A			; 行番号0ならERR
	BNE SL3
ERRS	LDA B	#3
	FCB	$8C
ERR2	LDA B	#2
	JMP	ERR
*
SL2	BSR	NXTL1
RS1	INX
SL3	JSR	CPBA
	BHI	SL2
RTN1	RTS
*
SUBIF	JSR	CLCPL		; IF,UNTILの
	TST B			; 　条件判断
	BNE	SUBIF0
	TST A
SUBIF0	RTS
*				;"IF"
IF	BSR		SUBIF
	BEQ		REM
	BRA		RN2
*
SIZE	LDA A	MEMEND+1	; 残りメモリー計算
	LDA B	MEMEND
	SUB A	EOP+1
	SBC B	EOP
	RTS
*
END2	JMP	END1
*				; [実行モード]
RUN	BSR	INITP
	CLR	DIRECT
	LDX	BOP
	INX
RN1	STX	EADRS		; TEXTの終わりまで行った。EADRS←行番号の位置
	TST	DIRECT
	BNE	END2
	LDA A	0,X		; 　時 又はDIRECT MODEの時は終わる
	BMI	END2
RN12	INX			; 行番号スキップ
	INX
RN2	JSR	BRK
	JSR	CHVAR		; 変数チェック
	BCS	RN31
RN3	JSR	LET		; 代入文処理
	JSR	PKUP
	BEQ	RN6
ERR18	LDA B	#18
	FCB	$8C
ERR19	LDA B	#19
	JMP	ERR
*
RN31	LDX	#STATE		; ステートメント処理
RN4	LDS	#STACK
	JSR	TBLJP
RN5	JSR	PKUP
	BNE	ERR19
RN6	INX
	BCC	RN1		; 行末 00
	BRA	RN2		; ':' マルチステートメント
*				; 16進入力処理
HB1	ROL A
	ASL A
	ASL A
	ASL A
	ASL A
HB2	ROL	WKC
	ROL B
	ASL A
	BNE	HB2
HTOB	INX
	BSR	TSTHEX
	BCS	HB1
	LDA A	WKC
	RTS
*
TSTHEX	JSR	TSTN		; 16進チェック
	BCS	RTN2
	SUB A	#7
	CMP A	#'9'+1
	CLC
	BLT	RTN2
	CMP A	#'@'
RTN2	RTS
*
CPBA	CMP B	0,X		; 2バイト比較
	BNE	CPBAE		; 　行末→               Z=1 C=0
	CMP A	1,X		; 　マルチステートメント Z=1 C=1
CPBAE	RTS			; 　代                   Z=0 C=1
*
TSF	STX	XS
	LDX	#FRNEXT		; (TABLE SERCH)
TBLSCH	STS	SPS		; 　SP←TEXT
	SEI			; 　IX←TABLE
	JSR	NMISET
TS1	LDS	XS
	DES
	INX
TS2	INX
	PUL A
	TAB
	SUB B	0,X
	ASL B
	BEQ	TS3		; if １文字等しい
TS21	LDA B	0,X		; 省略形処理
	INX
FB	BPL	TS21
	CMP A	#'.'
	BEQ	TS4
	COM B
	BNE	TS1		; if TABLE末でないなら
	BRA	TS5
*
TS3	BCC	TS2
				; 一致したものが
	INX			; 　見つかった時
TS4	INS					
	STS	XS
TS5	LDS	SPS
	JSR	NMICLR
	CLI
	RTS
*
PRINT	CLR	WKUSE		; USING FLAG CLR
	BSR	PK1		; 1 BYTE PICKUP
	BEQ	PR4		; if 文末なら
PR1	BSR	PR7
	BEQ	PR2		; if 2つ目の ' or " なら
	STX	XS
	LDX	#FMT		; FORMATTER
	JSR	TBLJP		; 　TABLE JUMP
PR2	BSR	PK1
	BEQ	PR4
	CMP A	#';'		; ';' なら
	BEQ	PR3
	CMP A	#','		; ',' なら
	BNE	ERR4
	BSR	PR6
PR3	INX
	BSR	PK1
E1	BNE	PR1
	RTS
*
PK1	JMP	PKUP
*
PR4	JMP	CR
*
PR5	JSR	OUTS		; ',' 処理
PR6	LDA A	CHCT
	BIT A	#7
	BNE	PR5
	RTS
*
CHR	JSR	CLCPL		; CHR$処理
AOT	JMP	OUT
*
TAB1	JSR	CLCPL		; TAB処理
	TAB
	BRA	TAB2
*
TAB11	JSR	OUTS
TAB2	CMP B	CHCT
	BHI	TAB11
	RTS
*
PR7	BSR	PK1		; リテラル・ストリング処理
	CMP A	#'"'
	BEQ	PR71
	CMP A	#$27		; ''''
	BNE	RTN3
PR71	TAB
	INX
	BRA	PR81
*
PR8	BSR	AOT
PR81	LDA A	0,X
	BEQ	ERR4
	INX
	CBA
	BNE	PR8
RTN3	RTS
*
ERR4	LDA B	#4
	FCB	$8C
ERR5	LDA B	#5
	JMP	ERR
*				;除算
DIV	LDA A	0,X		; 除数=0ならERR
	ORAA	1,X
	BEQ	ERR5
	BSR	SETCT
	BSR	MDV3
	ROL A
	NEG A
	TAB
	BSR	MDV4
	ROL A			; Cフラグ反転
	EOR A	#1
	ROR A
	BSR	MDV2
	STA A	MDS+1		; 余りSAVE
	STA B	MDS
	JSR	CPBA
	BNE	DV1
	CLR A
	TAB
	INC	3,X
	BNE	DV1		; 値 M(2,X) M(3,X)
	INC	2,X
DV1	INX
	INX
	RTS
*
MDIV	BSR	SETCT		;（割り算サブルーチン）
MDV1	BSR	MDV3		; (IX+2)←(IX+2)/(IX)
	ROL A			;       <-------------
	ROL B			;  |  D   |  A   |M(2,X)|M(3,X)|
	SUB A	1,X		;  |M(0,x)|M(1,X)|  （被除数)
	SBC B	0,X		;    (除数子）
	BSR	MDV4		;			↓
MDV2	DEC	LPCT		;  |  B   |  A   | 余り.
	BNE	MDV1
MDV3	ROL	3,X
	ROL	2,X
	RTS
*
MDV4	PSH B			; 除数処理
	EOR B	0,X
	PUL B
	SEC
	BPL		RTN4
ADD	ADD A	1,X
	ADC B	0,X
	CLC
RTN4	RTS
*
SETCT	LDA A	#16
	STA A		LPCT
	CLR A
	CLR B
	RTS
*				;（乗算）
MULTI	BSR	SETCT
MLT1	ASL A
	ROL B			;  |M(0,X)|M(1,X)| * |M(2,X)|M(3,X)|
	ROL	1,X		;        ↓
	ROL	0,X		;  |  D   |   A  |
	BCC	MLT11		;        ↓
	ADD A	3,X		;  |M(2,X)|M(3,X)|
	ADC B	2,X		;        ↓
MLT11 	DEC	LPCT		;		INX
	BNE	MLT1		;		INX
	INX
	INX
	JMP	STBA
*				;（数字出力）
PNUM	LDX	0,X		; 数（BINARY）を転送
	STX	WKB
	LDX	#WKB
	BSR	ABSX		; 絶対値
	BCS	PU1		; 　if 負数なら
	LDA B	WKUSE		; USING処理
	BEQ	PU2
	TBA
	FCB	$8C
PU1	LDA A	#'-'		; '-'出力
	BSR	OUT
PU2	DEX
	DEX
	LDA B	#10		; 除数10 SET
	STA B	1,X
	CLR	0,X
	ASR B			; LOOPCOUNTER 5
PN1	PSH B			; 　5回10で割る
	BSR	MDIV
	PUL B
	PSH A			; ・余りPUSH
	DEC B
	BNE	PN1
 	LDX	#BFFR
	LDA B	#4		; LPCT最初○
PN2	LDA A	LPCT
	PUL A			; ・余りPUL
	BNE	PN3		; 0サプレス
	STA A	LPCT
	BNE	PN3
 	TST	WKUSE		; USING処理
	BEQ	PN4
	LDA A	WKUSE+1
	SUB A	#'0'
PN3	BSR	PN5		; ASCII変換して出力
PN4	DEC	B
	BNE	PN2
	PUL A			; 余りPUL 最後の桁は0サプレスしない
PN5	ADD A	#'0'		; ASCII変換して出力
 	TST	FAUT
	BEQ	OUT
	STA A	0,X
	INX
OUT	INC	CHCT
OUTC	JSR	BRK
 	JMP	OUTEEE
*
ABSX	TST	0,X		; 符号チェックして絶対値を
	BPL	RTN5		; 　とる（正 C=0）
NEGX	NEG	1,X		;       （負 C=1）
	BNE	NEGX1		; 符号反転
	DEC	0,X
NEGX1	COM	0,X
RTN5	RTS
*
CHASC	CMP A	#'@'		; ASCII CHECK
	BCS	CHV1		; 'A'〜'Z' →C=1
	CMP A	#'Z'+1		; 他       →C=0
	RTS
*
CHV1	CLC
	RTS
*
CHVAR	BSR	PKUP		; 変数チェック
	STX	XS		; TABLEへ行く必要の
	CMP A	#'!'		; 　あるもの C=1
	BEQ	TBL		; 　ないもの C=0
	CMP A	#'.'
	BEQ	TBL
	CMP A	#'*'
	BEQ	TBL
	BSR	CHASC
	BCC	CHV1
	LDA A	1,X
	CMP A	#'.'
	BEQ	TBL
	BSR	CHASC
	BCC	CHV1
	RTS
*				;(PICKUP)
ICPKUP	INX
PKUP	LDA A	0,X		; スペース読み飛ばす
	CMP A	#' '
	BEQ	ICPKUP		; 区切りチェック
	TST A			; 区切り $00 Z=1 C=0
	BEQ	RTN6		; 区切り ':' Z=1 C=1
	CMP A	#':'		; 他         Z=0 C=1
TBL	SEC
RTN6	RTS
*				;[第2レベル演算]
EX1	BSR	PKUP
	CMP A	#'-'
	BNE	EX2
	BSR	EX7		; マイナス処理
	BRA	EX4
EX2	CMP A	#'+'		; プラス&符号なし処理
	BEQ	EX3
	DEX
EX3	BSR	TM1
EX4	LDX	XS
	BSR	PKUP
	CMP A	#'+'
	BNE	EX6
	BSR	TM1		; 加算
EX5	BSR	EX8
	BRA	EX4
EX6	CMP A	#'-'
	BNE	RTN7
	BSR	EX5
	BRA	EX5
*
EX7	BSR	TM1
 	BRA	NEGX
*
EX8	JSR	CPUL		; 算術スタック内の
	INX			; 　加算サブルーチン
	INX
ADBA	ADD A	1,X
	ADC B	0,X
STBA	STA A	1,X

	STA B	0,X
RTN7	RTS
*
ABS	JSR	CUL1		; "ABS"
	BSR	TM2
	JSR	ABSX
	LDX	XS
	RTS
*
EX11	ORAB	#1		; 比較論理演算
	FCB	$8C		; 　チェック
EX12	ORAB	#2
	FCB	$8C		; 結果AccB
EX13	ORAB	#4		; >  0000 0001
	INX			; =  0000 0010
	FCB	$C1		; <  0000 0100
EX14	CLR B			; >= 0000 0011
	BSR	PKUP		; <= 0000 0110
	CMP A	#'>'		; <> 0000 0101
	BEQ	EX11
	CMP A	#'='
	BEQ	EX12
	CMP A	#'<'
	BEQ	EX13
	CLR A
	RTS
*
TM1	BSR	TERM
TM2	STX	XS
	LDX	CSP
RTN8	RTS
*				;（第3レベル演算）
EXPR	BSR	EX1
	BSR	EX14		; 論理演算チェック
	TST B
	BEQ	RTN8
	PSH B
	BSR	EX1
	JSR	CPUL		; 演算（再帰的）
	LDX	CSP		; BAは後の数
	SUB A	1,X		; 比較
	SBC B	0,X		; 後-前
	PUL B
	BLT	LT		; 論理演算
	BGT	GT
	TST A
	BEQ	EQ
GT	ASR B
EQ	ASR B
LT	AND B	#1
	STA B	1,X
	CLR	0,X
	LDX	WKC
	RTS
*				;(ASCI→BINARY)
ATOB	BSR	TSTN		; 10進チェック
 	BCC	RTN9
	CLR B
	STA B	WKC+1
ATB1	STA B	WKC
	AND A	#$F
	STA A	LPCT
	LDA A	WKC+1
	ASL A
	ROL B
	ASL A
	ROL B
	ADD A	WKC+1		; 数字のつづく限り
	ADC B	WKC		; 　BINARY変換
	ASL A
	ROL B
	ADD A	LPCT
	STA A	WKC+1
	ADC B	#0		; 結果 |B|A|
	INX
	BSR	TSTN		
	BCS	ATB1		; 数字なら C=1
	LDA A	WKC+1		; 他       C=0
	SEC
RTN9	RTS
*				;（10進チェック）
TSTN	JSR	PKUP
	CMP A	#'0'		; 10進 C=1
	CLC			; 他   C=0
	BLT	TSTN1
	CMP A	#':'
TSTN1	RTS
*				;[第1レベル演算]
TERM	BSR	FUNCT
TR1	JSR	PKUP
	CMP A	#'*'
	BNE	TR3
	BSR	TR4		; 乗算処理
	JSR	MULTI
TR2	STX	CSP
	LDX	XS
	BRA	TR1
TR3	CMP A	#'/'
	BNE	RTN9
	BSR	TR4		; 除算処理
	JSR	DIV
	BRA	TR2
*
TR4	BSR	FUNCT
	JMP	TM2
INHEX	CLR B			; '$' 16進入力
	STA B	WKC
	JSR	HTOB		; BINARY変換
	BRA	CPS2		; 算術スタックPSH
*				;'#' PEEK
PEEK	BSR	FUNCT
	JSR	CPUL		; 引数ADRS計算
	BSR	TBAX		; |BA|→IX
	LDA A	0,X
	CLR B
	BRA	CPS1		; 算術スタックPSH
*
LITRL	CMP A	2,X		; リテラル定数
	BNE	ERR7		; 　（ASCII定数）処理
	LDA A 1,X
	CLR B
	INX
	INX
	INX
	BRA	CPS2
*				;(関数TABLE JUMP)
FUNCT	INX
	BSR	ATOB		; ASCI→BINARY
	BCS	CPS2		; if 数字なら
	CMP A	#'#'		; if PEEK
	BEQ	PEEK
	CMP A	#'"'		; if リテラル
	BEQ	LITRL
	CMP A	#$27		; ''''
	BEQ	LITRL
	CMP A	#'$'		; if 16進数なら
	BEQ	INHEX
	JSR	CHVAR		; 変数チェック
	BCC	CUL		; if 変数 （2項演算子なら）
	LDX	#FUNC		; それ以外なら
*
TBLJP	JSR	TBLSCH		; 関数TABLEをサーチしてJUMP
	LDA A	1,X		; ADRSをスタックにPSH
	PSH A			; 　してRTSでJUMP
	LDA A	0,X
	PSH A
	LDX	XS
	RTS
*
CPUSH	BSR	TBAX		; BA←(BA)
    	LDA A	1,X
	LDA B	0,X
CPS1	LDX	XS
CPS2	STX	WKC
	LDX	CSP		; 算術スタックポインタ
	DEX
	DEX
	CPX	EOB
	BLE	ERR6
CPS3	JSR	STBA
	STX	CSP
CPS4	LDX	WKC
	RTS
*
ERR6	LDA B	#6
	FCB	$8C
ERR7	LDA B	#7
	FCB	$8C
ERR8	LDA B	#8
	JMP	ERR
*
CUL	BSR	VAR		; 変数、IX変数、IX配列のADRS計算
	BCC	CPUSH		; if '(' なら
CUL1	LDA A	0,X

	CMP A	#'('		; '(' 〜 ')' の処理
	BNE	ERR7
CUL2	INX
CUL3	JSR	EXPR		; 演算（再帰的）
CUL4	LDA A	0,X
	CMP A	#')'
	BNE	ERR7
	INX
	RTS
*
TBAX	STX	XS		; IX←BA
	STA A	WKC+1
	STA B	WKC
	BRA	CPS4
*				;"MOD"
MOD	JSR	SUB5
	BEQ	MOD1
	LDA B	MDS		; 引数なしの時
	LDA A	MDS+1
	BRA	CPS2
*
MOD1	INX			; MOD(A,B)
	JSR	EXPR		; 引数付の時	
	BSR	COMMA		; 2つの引数を算術スタックにPSH
	BNE	ERR8
	INX
	BSR	CUL3
MOD2	STX	WKC
	LDX	CSP
	JSR	DIV		; 割算して余りを得る
	BRA	CPS3
*
COMMA	LDA A	0,X
	CMP A	#','
	RTS
*				;"RND"
RND	LDA B	RNDS+1		; 乱数レジスタ変更
	LDA A	RNDS
	ABA
	ADC B	#$95
	ADC A	#$AB
	STA A	RNDS+1
	STA B	RNDS
	BSR	CPS2		; 算術スタックPSH
	BSR	CUL1		; 引数計算
	BRA	MOD2		; 割算して余りを乱数とする
*				;"USER"
USER	JSR	SUB10
	BSR	US2
	BRA	CPS1		; ⇦機械語からはここへRETURN
US2	BSR	CCP1		; 第1引数
	PSH A
	PSH B
	BSR	US3		; 第2引数
	PSH A
	PSH B
	BSR	US3		; 第3引数
	PSH A
	PSH B
	TPA
	PSH A
	BSR	CUL4
	STX	XS
	RTI
*
US3	BSR	COMMA
	BNE	US4
	INX
CCP1	JMP	CLCPL
*				; 引数がない時0にする’
US4	CLR A
	CLR B
NOTV	SEC
	RTS
*				;変数、IX変数、IX配列
VAR	JSR	PKUP		;　ADRS計算
	CMP A	#'%'		; IX変数 or IX配列
	BEQ	ARR
	CMP A	#'@'		; 単純変数チェック
	BCS	NOTV
	CMP A	#']'
	BHI	NOTV
	AND A	#$3F		; 変数ADRS
	ASL A
	LDA B	#VTOP/256
	INX
	RTS
ARR	INX			; IX変数 IX配列
	JSR	TSTN		; 10進チェック
	BCC	ERR14
	INX
	JSR	SUB5
	BEQ	IND
DIR	AND A	#$F		; IX変数 ADRS計算
	ASL A
	CLR B
	ADD A	#IXV
	ADC B	#IXV/256
	RTS
*
IND	PSH A			; ※配列&スタックPSH
	JSR	CUL2		; 引数演算（再帰的）
	PUL A			; ※
 	BSR	DIR		; ADRS計算
	JSR	TBAX		; 引数を算術スタックからPUL
	JSR	CPUL1
	ASL A
	ROL B
	JSR	ADD		; ADRS加算
	LDX	XS
	RTS
*				;（代入文処理）
LET3	BSR	VAR
	BCS	ERR9		; 左辺ADRS SAVE
LET4	STA A	WKB+1
	STA B	WKB
	LDA A	0,X
	CMP A	#'='
	BNE	ERR9
	INX
	BSR	CLCPL		; 右辺計算（再帰的）
	STX	XS
	LDX	WKB
	RTS
*
ERR14	LDA B	#14
	FCB	$8C
ERR9	LDA B	#9
	FCB	$8C
ERR10	LDA B	#10
	JMP	ERR
*
THEN	CLR	ADP+3
	JSR	ATOB		; 行番号ならGOへ
	BCS	GO1
 	JMP	RN2		; 他ならステートメント処理
*				;"POKE" "#"
POKE	JSR	FUNCT		; ADRS計算（再帰的）
	BSR	CPUL
	BSR	LET4		; 代入処理
	TAB
	BRA	LET2
*				;（IXスタックPSH）
PSHX	STS	SPS
	SEI
	JSR	NMISET
	TXS
	LDX	XSP		; IXスタックポインタ
	CPX	#STACK+3
	BEQ	ERR10
	DEX
	DEX
	STS	0,X
PX1	STX	XSP
	TSX
	LDS	SPS
	JSR	NMICLR
	CLI
	RTS
*				;（IXスタックPUL）
PULX	LDX	XSP
	CPX	#IXSTCK
	BEQ	ERR10
	STS	SPS
	SEI
	JSR	NMISET
	LDS	0,X
	INX
	INX
	BRA	PX1
*
CLCPL	BSR	EXPR2		; 演算して結果 |B|A|
CPUL	STX	WKC		; 算術スタックPUL
CPUL1	LDX	CSP		; 算術スタックポインタ
	CPX	#CSTCK
	BEQ	ERR9
	LDA A	1,X
	LDA B	0,X
	INX
	INX
	STX	CSP
	LDX	WKC
RTN13	RTS
EXPR1	LDX	XS
EXPR2	JMP	EXPR

*				;"GO"
GOTO	CLR A			;"GOSUB"
GOSUB	STA A	ADP+3
	BSR	CLCPL		; 演算
GO1	JSR	SETLN		; 行番号SET
	TST B
	BNE	GO2
	TST A
	BEQ	RTN13		; 飛び先が行番号0なら何もしない
GO2	TST	ADP+3		; GUSUBなら
	BEQ	GO3	    	; IXスタックPSH
	BSR	PSHX
;GO21	LDX	EADRS		; 飛び先が現在の行
;	JSR	CPBA		; 　より前か後か
;	BCC	GO3		; AccBA>=IX(0,1)
;	ROR	EADRS
GO3	JSR	SCHEND		; 行番号サーチ
 	BCS	ERR11
	JMP	RN1
*				;（代入文）
LET	CMP A	#'#'
 	BEQ	POKE
LET1	JSR	LET3
	STA A	1,X		; 代入
LET2	STA B	0,X
	LDX	XS
	RTS
*				;"FOR"
FOR	BSR	LET1		; 制御変数に初期パラメータ代入
	LDA A	WKB+1		; 制御変数のアドレス
	LDA B	WKB
	JSR	CPS2		; 算術スタックPSH
	JSR	TSF		; 'TO'チェック
	CPX	#TO
	BNE	ERR12
	BSR	EXPR1		; 最終パラメータ計算→PSH
	JSR	TSF		; 'STEP'チェック
	CPX	#STEP
	BEQ	FOR1
	LDA A	#1		; STEPなしなら 1にする
	CLR B
	JSR	CPS1
	BRA	FOR2
*
FOR1	BSR	EXPR1		; STEP計算→PSH
	STX	XS
FOR2	LDA A	XS+1		; ループの戻りADRS PSH
	LDA B	XS
	JSR	CPS2
	LDA A	WKB+1		; 制御変数ADRS
	LDA B	WKB
	LDX	FSP		; FORNEXTスタックポインタ
	CPX	#FSTCK		; ネスティング0なら
	BEQ	FOR5
	BSR	TSTPM		; パラメータチェック
	BCC	FOR4
	LDX	FSP
 	CPX	#CSTCK
	BNE	FOR5
ERR12	LDA B	#12
	FCB	$8C
ERR11	LDA B	#11
	FCB	$8C
ERR13	LDA B	#13
	JMP	ERR
*
FOR4	BSR	INX8		; 算術スタックから
FOR5	BSR	FORPSH		; FOR NEXT STACKへ転送
 	BRA	NXT3
*
TSTPM	JSR	CPBA		; テストパラメータ 2バイト比較
	BEQ	RTN11		; 　他のすべての FOR NEXT LOOP のパラメータと比較
 	BSR	INX8	; →ループ内飛びだしの調整
TPM1	CPX	#FSTCK		; 　（同じものがない C=1）
	BNE	TSTPM		; 　（同じものがある C=0）
	SEC	
RTN11	RTS
*
FORPSH	BSR	FORPSH2		; 8バイト算術スタックからIXの示すところへ転送
FORPSH2	BSR	FORPSH3
FORPSH3	JSR	CPUL
	DEX
	DEX
	JMP	STBA
*				;"NEXT"
NEXT	JSR	VAR		; 制御変数のアドレス計算
	STX	XS
	LDX	FSP		; ←FOR NEXT スタックポインタ
	BCC	NXT1
	LDA A	1,X		; 制御変数なしなら自動的に決定
	LDA B	0,X
NXT1	BSR	TPM1		; パラメータチェック
 	BCS	ERR13
	STX	FSP
	LDA A	5,X		; STEPの値得る
	LDA B	4,X
	LDX	0,X		; パラメータADRS
	JSR	ADBA		; STEPを加算
	LDX	FSP
	CMP B	2,X		; 最終パラメータと上値比較
	SEC			; if 上値が等しくない
	BLT	NXT2		; 　正 C=0
	CLC			; 　負 C=1
	BGT	NXT2
	CMP A	3,X
 	BEQ	NXT4		; if 上値が等しい
NXT2	ROR A			; LOOPの符号を考慮して
	EOR A	4,X
 	BMI	NXT4		; if 満たされてない
	BSR	INX8		; LOOP 終了
NXT3	STX	FSP
	LDX	XS
	RTS
*
INX8	INX
	INX
	INX
	INX
	INX
	INX
	INX
	INX
	RTS
*
NXT4	LDX	6,X		; FORの次のTEXTのADRS
	RTS
*
INPUT	STX	WKB
	LDA A	WKB		; DIRECT MODEならERR
	BEQ	ER1
	LDX	EOB		; 入力バッファ最終値
	STX	WKA
	LDX	WKB
	JSR	PR7		; リテラルストリングPRINT
	BNE	INP1
	BSR	INP3
INP1	JSR	VAR		; 入力変数ADRS計算
 	BCS	ER1		; if 変数、IX変数、IX配列以外なら
	PSH A			; ADRS PSH
	PSH B
	STX	WKB
	LDX	WKA
	BSR	PKUP1
	BNE	INP2		; if 入力変数が？？くなる
INA	LDA A	#'?'		; '? ' 出力
	BSR	OUT1
	BSR	OUTS
	JSR	LN4		; 入力バッファIN
INP2	JSR	PKUP
	CMP A	#'$'		; 入力MISSチェック
	BCS	MISS
	CMP A	#'['
	BCS	OK
MISS	STX	WKC		; MISS なら
	LDX	#REENT		; 　RE-ENTER を出力して
	JSR	MGOT		; 　入力やりなおし
	BRA	INA
*
OK	JSR	CLCPL		; 式の演算
	BSR	INP3
	STX	WKA
	TSX			; PSHしたADRSをIXにLOAD
	LDX	0,X
	INS
	INS
	JSR	STBA
	LDX	WKB
	BSR	PKUP1
	BEQ	RTN12
	CMP A	#','		; ','なら
	BNE	ER1		; もう一変数入力
	INX
	BRA	INP1
*
ER1	JMP	ERR1
*
INP3	BSR	INP4
 	BNE	RTN12
	INX
RTN12 RTS
*
INP4	PSH A
	BSR	PKUP1
	CMP A	#','
	PUL A
	RTS
*
PKUP1	JMP	PKUP
*
OUTS	LDA A	#' '
OUT1	JMP	OUT
*
DDLY	BSR	DELAY		    ; 1秒DELAY
DELAY	STX	WKC		    ; 0.5秒 DELAY
	LDX	#$E100
DEL1	DEX
	BNE	DEL1
	LDX	WKC
RTN10	RTS
*
SAVE1	INX
SAVE	LDA	0,X
	CMPA	#' '
	BEQ	SAVE1
;	LDA A	#$12		    ; 出力をCASETに設定
;	BSR	OUT1
;	STA A	FS
;	BSR	DDLY
	LDA B	#6
	CLR	LSWK2		    ; Filename -> $0043
	LDA A	#$43
	STA A	LSWK2+1
SAVE2	LDA A	0,X
	BEQ	SAVE3
	CMP A	#' '
	BEQ	SAVE3
	CMP A	#':'
	BEQ	SAVE3
	INX
	STX	LSWK
	LDX	LSWK2
	STA A	0,X
	INX
	STX	LSWK2
	LDX	LSWK
	DEC B
	BNE	SAVE2
	BRA	SAVE4
SAVE3	STX	LSWK
	LDX	LSWK2
SAVE31	LDA A	#' '
	STA A	0,X
	INX
	DEC B
	BNE	SAVE31
SAVE4	LDA A	#'.'
	STA A	$49
	LDA A	#'S'
	STA A	$4A
	STA A	FS		    ; SAVE Flag 0 or not 0
	CLR	$004C
	LDX	#IOBUF
	STX	$003B
	STX	$003D
	LDX	LSWK
	BRA	LST1
*
SHOOK	STX	LSWK2
	LDX	$003D
	STA A	0,X
	INX
	STX	$003D
	CPX	#IOBUF+256
	BNE	SHOOK9
	BSR	SHSUB
SHOOK9	LDX	LSWK2
	RTS
SHSUB	STX	LSWK3
	PSH B
	PSH A
	INC	$004C
	JSR	BYTOUT
	LDA A	#'-'
	BSR	ASCOUT2
	LDA A	#' '
	BSR	ASCOUT2
	LDA A	$004C
	BSR	OUTHEX
	LDA A	#$0D
	BSR	ASCOUT2
	LDX	#IOBUF
	STX	$003B
	STX	$003D
	PUL A
	PUL B
	LDX	LSWK3
	RTS
ASCOUT2	JMP	ASCOUT
*				; print decimal one byte
OUTHEX	PSH A
	LSR A
	LSR A
	LSR A
	LSR A
	BSR OH1
	PUL A
OH1	PSH A
	CMP A	#10
	BCS	OH2
	ADD A	#7
OH2	ADD A	#'0'
	BSR	ASCOUT2
	PUL A
	RTS
*
LIST	CLR	FS
LST1	CLR B
	STA B	WKUSE
	LDA A	#1		    ; 行番号を1にする
	JSR	SETLN
	LDA B	#$7F		    ; 最終行番号
	LDA A	#$FF
	PSH A
	JSR	PKUP1		    ; 引数チェック
	PUL A
	BEQ	LST2		    ; if 引数なし
	JSR	CLCPL		    ; 第1引数
	JSR	SETLN
	JSR	INP4
	BNE	LST2		    ; if 第2引数なし
	INX			    ; 第2引数
	JSR	CLCPL
LST2	STA A	SPS+1		    ; 最終行番号
 	STA B	SPS
	LDX	SPS
	BGT	LST3		    ; 第2引数が0または負なら
	LDX	#$7FFF		    ; 最後までリスト
	STX	SPS
LST3	JSR	SCHLIN
	FCB	$8C
*
LST4	BSR	LST5		    ; 最終行番号になるまでLIST
	LDA A	SPS+1
	LDA B	SPS
	JSR	CPBA
	BCC	LST4
;CNTCRD	STX	WKC		    ; CASET入出力解除
;	LDX	#CNT		    ; のCONTROL CORD出力
;	BSR	MGOT
	TST	FS
	BEQ	TOEND
	PSH A
	CLR A			    ; SAVE末に00出力
	JSR	SHOOK
	PUL A
	LDX	$003B		    ; SAVE残りあり？
	CPX	$003D
	BEQ	TOEND
	JSR	SHSUB
TOEND	CLR	FS
	JMP	END		    ; エディットモードにもどる
*				    ;（リスト処理サブルーチン）
LST5	;LDA A	#2		    ; 行の頭を示すCONTROL CORD出力
 	;BSR	OUT1
	BSR	LST7		    ; 行番号出力
	INX
 	LDA A	#' '		    ; スペースに続いて
	BSR	MGOT1		    ; 　TEXT 1行出力
	BSR	CR		    ; 'CR'出力
	LDA A	FS		    ; SAVEならDELAYを置く
 	BEQ	LST51
	JMP	DELAY
LST51	RTS
*
CR	STX	WKC
	LDX	#CRLF
MGOT	BSR	MGOUT
	CLR	CHCT
LST6	LDX	WKC
	RTS
*
LST7	STX	WKC
LST8	JSR	PNUM		    ; BINARYをASCII化して出力
	BRA	LST6
*
MGOT1	JSR	OUT		    ; IXの示す所から$00
	INX			    ; 　まで出力する
MGOUT	LDA A	0,X
	BNE	MGOT1
	INX
	RTS
*
ERR	LDS	#STACK		    ; SPの初期化
	CLR	FLOD
	CLR	FS
	BSR	CR		    ; 改行して
	LDX	#ERROR		    ; 　'ERROR NO.'を出力
	BSR	MGOUT
	TBA			    ; ERROR NUMBERを
	CLR B			    ; 　出力
	STA B	WKUSE
	BSR	CCP3
ERRBRK	LDX	EADRS
	TST	DIRECT
	BMI	TOEND		    ; DIRECT MODE 以外なら
	CPX	#BFFREND
	BMI	TOEND
	BSR	CR		    ; 　ERRORの行を
	BSR	LST5		    ; 　LISTする.
	BRA	TOEND
*
USING	LDA B	0,X		    ; USING処理 
	STA B	WKUSE
	LDA B	1,X
	STA B	WKUSE+1
	INX
	INX
*
CCP2	JSR	CLCPL		    ; 式の値演算
CCP3	PSH A			    ; |BA|の値を
	PSH B			    ; 　ASCII化して出力
	STX	WKC
	TSX
	BSR	LST8
	PUL B
	PUL A
 	CLR	WKUSE
	RTS
*
OHL	LSR A			    ; 上位4bitの16進出力
	LSR A
	LSR A
	LSR A
OHR	AND A	#$F		    ; 下位4bitの16進出力
	CMP A	#$A
	BCS	OHR2
	ADD A	#7
OHR2	ADD A	#'0'
TOOUT	JMP	OUT
*
O4H	PSH A			    ; 2byte 4桁の16進出力
	TBA
	BSR	O2H
	PUL A
O2H	PSH A			    ; 1byte 2桁の16進出力
 	BSR	OHL
	PUL A
	BRA	OHR
*				    ;"HDT " & "HDF"
HD	LDA A	#'$'		    ; '$'出力
	BSR	TOOUT
	LDA A 0,X
	INX
	CMP A	#'T'
	BEQ	HDT
HDF	JSR	CLCPL		    ; HDF処理
	BSR	O4H
TOOUTS	JMP	OUTS
*
HDT	JSR	CLCPL		    ; HĐT処理
 	BSR	O2H
	BRA	TOOUTS
*
GET	JSR	INEEE		    ; "GET$" 関数
	CLR B
GET1	JMP	CPS2
*				    ;(OPTION) "COPY"ステートメント
;COPY	JSR	VRAMPR
;	BRA	G1
*
* GRAPHIC			    ;［グラフィック命令］
*
;NEGB	JMP	NEG		    ; "NEG"
PICKB	BSR	SUB2		    ; "!P(X,Y)" 関数
	BSR	SUB4
	JSR	PICK
GE1	JMP	CPS2
*				    ;"!W(X,Y)"ステートメント
SETPT	LDA A	0,X
	CMP A	#'W'
	BNE	G2
	BSR	SUB3
	PSH A
	PSH B
	LDA A	#1
	STA A	WKA
G0	JMP	PLOT0
*				    ;"!B(X.Y)"ステートメント
G2	CMP A	#'B'
	BNE	G3
	BSR	SUB3
	PSH A
	PSH B
	CLR	WKA
	BRA	G0
*				    ;"!R(X,Y)"ステートメント
G3	CMP A	#'R'		    
 	BNE	ERR15
	BSR	SUB3
	PSH A
	PSH B
	TPA
	STA A	WKA
	BRA	G0
*
SUB5	LDA B	0,X
	CMP B	#'('
	RTS
*
SUB3	BSR	SUB1
SUB4	CMP A	#GWIDTH		    ; GRAPHIC
 	BCC	ERR16		    ; 　引数範囲の
	CMP B	#GHEIGHT	    ; 　チェック
	BCC	ERR16
	RTS
*
SUB1	INX
 	BSR	SUB5
	BNE	ER9
SUB6	INX
SUB2	JSR	SUB7		    ; 2引数
	ORAB	ADP		    ; 　第1引数 AccA
	BNE	ERR16		    ; 　第2引数 AccB
	TAB
	LDA A	ADP+1
	RTS
*
ERR15	LDA B #15
	FCB	$8C
ERR16	LDA B	#16
 	JMP	ERR
*				    ;CLR(X,Y) KEY(X,Y)
CMP	BCC	ERR16		    ; 引数範囲チェック
	CBA
	BGT	ERR16
	RTS
*				    ;"CLR(X,Y)"
CLR	BSR	SUB5
	BNE	ALLCLR		    ; if引数なし
	BSR	SUB6
	CMP B	#HEIGHT
	BSR	CMP
C1	PSH A
	BSR	C3		    ; 第2引数
	ADD B	#WIDTH
	ADC A	#0
	STA A	ADP+2
	STA B	ADP+3
	PUL B
	BSR	C3		    ; 第1引数
	STA A	ADP
	STA B	ADP+1
	LDA B	#' '		    ;⇦ SPACE CORD
	LDX	ADP		    ; ふつうのV-RAMでは$20
C2	STA B	0,X		    ; 　に変更
	INX
	CPX	ADP+2
	BNE	C2
CE	LDX	XS
	RTS
*
ALLCLR	JSR	CLRTV
	BRA	CE
*
C3	CLR A			    ; 引数からADRS計算
	ASL B
	ASL B
	ASL B
	ASL B
	ROL A
	ASL B
	ROL A
	ADC A	#VRAM/256	    ; V-RAM 開始ADRSの
	RTS			    ; 　上位1バイト
*
ER9	JMP	ERR9
*				    ;"CURS(X , Y)"
CURS	BSR	SUB10		    ;      ↓  ↓
				    ;	AccA   AccB
	BSR	SUB2		    ; 引数
	CMP A	#WIDTH		    ; CURSORの範囲チェック
	BCC	ERR16
	CMP B	#HEIGHT
	BCC	ERR16
	STA A	CURX
	STA B	CURY
	RTS
*				    ;"SGN(X)"関数
SGN	JSR	CUL1		    ; 引数計算
	JSR	CPUL		    ; 　|B|A|
	TST B
	BHI	MINS
	BNE	PLS
	TST A
 	BEQ	ZERO		    ; 正  →1
PLS	CLR B			    ; ゼロ→0
 	LDA A	#1		    ; 負  →-1
	BRA	ZERO
*
MINS	LDA A	#$FF
	TAB
ZERO	JMP	CPS2
*
ER7	JMP	ERR7
*
SUB10	JSR	SUB5
 	BNE	ER7
	INX
	RTS
*
SUB11	BSR	SUB10		    ; 1バイト以内の
SUB7	JSR	CLCPL		    ; 　2引数処理
	STA A	ADP+1		    ; 第1引数
	STA B	ADP
	JSR	COMMA
	BNE	ER9
	INX
	JSR	CLCPL		    ; 第2引数
	PSH A
	JSR	CUL4
	PUL A
	STX	XS
	RTS
*				    ;"AND(X,Y)"関数
AND	BSR	SUB11
	AND A	ADP+1
	AND B	ADP
AND1	JMP	CPS2
*				    ;"OR(X,Y)"関数
OR	BSR	SUB11
	ORAA	ADP+1
	ORAB	ADP
	BRA	AND1
*				    ;"XOR(X,Y)"関数
XOR	BSR	SUB11
	EOR A	ADP+1
	EOR B	ADP
	BRA	AND1
*				    ;"RESTORE"ステートメント
RES	JSR	CLCPL		    ; 引数計算
	STX	WKA
	LDX	BOP		    ; TEXT最初から行番号を
	JSR	RS1		    ; 　捜す
	BCS	ER11
	DEX
	BRA	RE4
*				    ;"READ"関数
READ	STX	WKA
	LDX	DP		    ; DATAポインタ
	JSR	PKUP		    ; 次のTEXT Pick up
	CMP A	#','		    ; ','なら次のDATA
	BEQ	RE3
	TST A			    ; 文末なら次のDATA文を
	BEQ	RE2		    ; 　捜す
	JMP	ERR1
*
RE1	JSR	NXTL1		    ; 次の文末を捜す
RE2	LDA	1,X		    ; TEXTの最後まで行った
	CMP A	#$80		    ; 　らERROR
	BEQ	ERR17
	LDA A	3,X
	INX
	CMP A	#'*'		    ; '*'でなかったら
	BNE	RE1		    ; 　次の文末を捜す
	INX
	INX
RE3	INX
	JSR	EXPR		    ; DATAの値を計算する
RE4	STX	DP
	LDX	WKA
	RTS
*				    ;"DEL"
DEL	BSR	RES		    ; 引数の行番号のADRS
	LDX	DP		    ; 　捜す
	INX
	JSR	DEL2		    ; その行から後をNEWと
	JMP	END		    ; 　同じ処理をする
*
ER11	JMP	ERR11
*
ERR17	LDA B	#17
	JMP	ERR
*				    ;"KEY"関数（注 HGF用）
KEY	JSR	SUB5		    ;※別記
	BEQ	K0
	LDA B	#7
	CLR A
	BRA	K1
*
K0	JSR	SUB6		    ; BM版では引数に意味はないが残しておく
	CMP B	#6
	JSR	CMP
K1	JSR	KBIN		    ; AccA←Code, C=1 if pressed
	BCC	K2
	LDA A	#100		    ; 100でいいのか？
K2	CLR B
	JMP	CPS2
*				    ;"UNTIL"
UNTIL	JSR	SUBIF		    ; 条件式の値
	BEQ	UNT		    ; if 不成立
	JSR	PULX		    ; 成立したら IX STACKを
	LDX	WKC		    ; 　PULして次の文へ
	RTS
*
UNT	LDX	XSP		    ; 不成立なら
	LDX	0,X		    ; 　IXSTACKの内容を
	INX			    ; 　取りだし、そのADRSへ
	RTS
*				    ;SAUE, LIST. 解除
;CNT	FCB	3,$13,$14,0	    ; 　のためのCONTROL CORD
CNT	FCB	0
*
ERROR	FCC	'ERROR NO.'
	FCB	7,0
*
READY	FCB	$D
	FCC	'READY'
;	FCB	7
	FCB	0
*
CRLF	FCB	$D,0
*
REENT	FCC	'RE-ENTER'
	FCB	7,0
*				    ;［注:TABLEの各KEY WORDの
Z	EQU	$80		    ; 　最後の文字の上位1bitに
*				    ; 　1を立てる
COMAND	EQU	*-2
	FCB	'L','I','S','T'+Z
	FDB	LIST
	FCB	'L','O','A','D'+Z
	FDB	LOAD
	FCB	'R','U','N'+Z
	FDB	RUN
	FCB	'E','X'+Z
	FDB	$F0B1		    ;⇦MONITORのADRS
	FCB	'A','U','T','O'+Z
	FDB	AUTO
	FCB	'N','E','W'+Z
	FDB	NEW
	FCB	'S','A','V','E'+Z
	FDB	SAVE
	FCB	'A','P','P','E','N','D'+Z
	FDB	APPEND
	FCB	'D','E','L'+Z
	FDB	DEL
	FCB	'M','O','N'+Z
	FDB	MONITOR
*
STATE	EQU	*-2
	FCB	'N','E','X','T'+Z
	FDB	NEXT
	FCB	'U','N','T','I','L'+Z
	FDB	UNTIL
	FCB	'G','O','T','O'+Z
	FDB	GOTO
	FCB	'G','O','S','U','B'+Z
	FDB	GOSUB
	FCB	'T','H','E','N'+Z
	FDB	THEN
	FCB	'F','O','R'+Z
	FDB	FOR
	FCB	'D','O'+Z
	FDB	PSHX
	FCB	'!'+Z		    ; GRAPHICステートメント
	FDB	SETPT
	FCB	'R','E','T'+Z
	FDB	PULX
	FCB	'I','N','P','U','T'+Z
	FDB	INPUT
	FCB	'I','F'+Z
	FDB	IF
	FCB	'P','R','I','N','T'+Z
	FDB	PRINT
	FCB	'C','L','R'+Z
    	FDB	CLR
	FCB	'C','U','R','S'+Z
	FDB	CURS
	FCB	'N','E','G'+Z
	FDB	NEGB
	FCB	'R','E','S','T','O','R','E'+Z
	FDB	RES
	FCB	'R','E','M'+Z
	FDB	REM
	FCB	'*'+Z			; DATA文
	FDB	REM
	FCB	'E','N','D'+Z
	FDB	END
	FCB	'S','T','O','P'+Z
	FDB	STOP
;	FCB	'C','O','P','Y'+Z	; OPTIONとして各自が
;	FDB	COPY			; 　自分のステートメントに変更可
	FCB	'P','L','O','T'+Z
	FDB	PLOT
	FCB	$FF			; 一致するステートメントがなかったら
	FDB	ER9			; 　ERROR NO.9
*
FUNC	EQU	*-2
	FCB	'R','N','D'+Z
	FDB	RND
	FCB	'R','E','A','D'+Z
	FDB	READ
	FCB	'A','B','S'+Z
	FDB	ABS
	FCB	'M','O','D'+Z
	FDB	MOD
	FCB	'!','P','('+Z
	FDB	PICKB
	FCB	'U','S','E','R'+2
	FDB	USER
	FCB	'A','N','D'+2
	FDB	AND
	FCB	'O','R'+Z
	FDB	OR
	FCB	'X','O','R'+Z
	FDB	XOR
	FCB	'S','G','N'+Z
	FDB	SGN
	FCB	'K','E','Y'+Z
	FDB	KEY
	FCB	'G','E','T','$'+Z
	FDB	GET
	FCB	$FF			; 一致する関数がなかったら
	FDB	ERR7			; 　ERROR NO.7.
*
FRNEXT	EQU	*-2
	FCB	'T','O'+Z
TO	FDB	0
	FCB	'S','T','E','P'+Z
STEP	EQU	*
*
FMT	EQU	*-2			;［FORMATTER］
	FCB	'C','H','R','$'+Z
	FDB	CHR
	FCB	'T','A','B'+Z
	FDB	TAB1
	FCB	'H','D'+Z
	FDB	HD
	FCB	'U','S','I','N','G'+Z
	FDB	USING
	FCB	$FF
	FDB	CCP2
*
BREAKM	FCB	$0D
	FCC	'*BREAK'
	FCB	$0D,0
*
STARTM	FCB	$0C
	FCC	'NAKAMOZU TinyBASIC/BM '
	FCC	'&DATETIME'
*
	FCB	$0D,0
*
* /BM subroutine
*
PLOT	JSR     SUB10
	JSR	CCP4		; 第1引数
	PSH A
	BSR	PLOT8		; 第2引数
	TAB			; B<-pos X
	PUL A			; A<-pos Y
	JSR	SUB4
	PSH A
	PSH B
	BSR	PLOT8		; 第3引数
	STA A	WKA
	JSR	CUL4
PLOT0	STX	XS		; SAVE IX
*
	PUL B			; 第2引数 Y→AccB
	PUL A			; 第1引数 X→AccA
	BSR	PADRS
	CMP B	#$0F
	BCS	PLOT3		; $10以上なら00扱い
	CLR B
PLOT3	BSR	PDOT		; PLOT先キャラクタのbit値計算
	LDA A	WKA		; 0:off? >0:on <0:reverse?
	BEQ	PLOTOFF
	BPL	PLOT5
	EOR B	WKA+1		; dot xor
	BRA	PLOT6
PLOTOFF	COM	WKA+1		; dot off
	AND B	WKA+1
	BRA	PLOT6
PLOT5	ORAB	WKA+1		; dot on
PLOT6	CLR A			; bit patternをdot値に戻す
	ADD B	#PLOTD1
	ADC A	#PLOTD1/256
	STA B	ADP+1
	STA A	ADP
	LDX	ADP
	LDA B	0,X
	LDX	WKB
	STA B	0,X		; PLOTする
	LDX	XS		; RESTORE IX
	RTS
*
PLOT8   JSR	COMMA
	BNE	PLOT9
	INX
CCP4    JMP	CLCPL
PLOT9	CLR A
	CLR B
	SEC
	RTS
*				; !P(X,Y): AccA:X, AccB:Y
PICK	STX	XS
	BSR	PADRS
	CMP B	#$0F
	BCC	PICK3		; $10以上なら100を返す
	BSR	PDOT
	AND B	WKA+1
	BEQ	PICK4
	LDA B	#1
	BRA	PICK4
PICK3	LDA B	#100
PICK4	TBA
	CLR B
PICK9	LDX	XS
	RTS
*				; PLOT先のグラフィックCHR(AccB)をdotに値してAccBで返す
PDOT	CLR A
	ADD B	#PLOTD0
	ADC A	#PLOTD0/256
	STA B	ADP+1
	STA A	ADP
	LDX	ADP
	LDA B	0,X		; PLOT先のキャラクタのbitパターン
	RTS
*				; AccA:X, AccB:Yからアドレスを得てXに入れる
*				; A:破壊、BはPLOT位置のキャラクタ
*				; WKAにX,Y位置のビットパターン(1,2,4,8)	; ちょっと冗長
PADRS	CLR	WKA+1
	INC	WKA+1
	LSR B			; Y dot位置
	BCC	PADRS1		; Yの偶奇により1か4にする
	ASL	WKA+1
	ASL	WKA+1
PADRS1	PSH A			; X位置保存
	JSR	C3		; Yからアドレス計算
	STA B	WKB+1
	STA A	WKB
	PUL B			; 第1引数 X→AccB
	LSR B			; X dot位置
	BCC	PADRS2		; Xの偶奇により2倍する(1->2,2->4)
	ASL	WKA+1
PADRS2	ADD B	WKB+1		; AccAB←PLOTアドレス
	ADC A	#0	    
	STA B	WKB+1		; WKB←
	STA A	WKB
	LDX	WKB
	LDA B	0,X		; PLOT位置のキャラクタLOAD
	RTS
*				; NEG命令
NEGB	STX	XS
	LDX	#VRAM-1
NEG01	INX
	CPX	#VRAMEND+1
	BEQ	PICK9
	LDA B	0,X
	CMP B	#$20
	BNE	NEG02
	CLR B
NEG02	CMP B	#$10
	BCC	NEG01
	LDA A	#$0D
	SBA
	AND A	#$0F
NEG03	STA A	0,X
	BRA	NEG01
*
*   ベーシックマスターのグラフィックキャラクタは素直な2進表現ではないので、変換テーブルが必要
*		00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
PLOTD0	FCB	$0,$7,$9,$4,$A,$D,$1,$E,$2,$5,$B,$6,$8,$F,$3,$C	; CHR->dot
PLOTD1	FCB	$0,$6,$8,$E,$3,$9,$B,$1,$C,$2,$4,$A,$F,$5,$7,$D ; dot->CHR
;PLOTN0	FCB	$D,$C,$B,$A,$9,$8,$7,$6,$5,$4,$3,$2,$1,$0,$F,$E	; NEG
*
	IF	&BASICROM
	ORG	$C000
	JMP	START
	ENDIF
;
; vim: set ts=8 noai:
;
