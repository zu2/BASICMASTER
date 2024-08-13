*
*	GAME68 subrourines for GAME68 compiler
*
*
* BM Monitor subroutine
*
USRNMI		EQU     $0000
USRIRQ		EQU     $0002
BREAKV		EQU     $0004
TIMERV		EQU     $0006
RAMEND		EQU     $0008       ; BM Monitor workarea
TIME1		EQU     $000A
TIME2		EQU     $000B
TIME3		EQU     $000C
CURX		EQU     $000F
CURY		EQU     $0010
ASCIN		EQU     $0028
ASCOUT		EQU     $002B
BYTIN		EQU     $002E
BYTOUT		EQU     $0031
RECTOP		EQU     $003B
LDTOP		EQU     $003B
MSTTOP		EQU     $003B
RECEND		EQU     $003D
LDEND		EQU     $003D
MSTEND		EQU     $003D
CPYTOP		EQU     $003F
CPYEND		EQU     $0041
FNAME		EQU     $0043
MONITOR		EQU     $F000
MUSIC		EQU     $F00C
MOVBLK		EQU     $F009
KBIN		EQU     $F00F
CHRGET		EQU     $F012
CHROUT		EQU     $F015
NMISET		EQU     $FFE6       ; inhibit NMI
NMICLR		EQU     $FFE9       ; allow NMI
CLRTV		EQU     $FFEF
CURPOS		EQU     $FFF5
*
LINBUF		EQU		$0500
LBUFSIZ		EQU		6+1			; 数値入力しかしないので '-'と5桁まで。末尾に00
*
			IF		0
*
*	AccAB+IX => IX
*
ADX			STX		ADXWK
			ADDB	ADXWK+1
			ADCA	ADXWK
			STAB	ADXWK+1
			STAA	ADXWK
			LDX		ADXWK
			RTS
*
*	AccAB*2+IX => OX
*
ADX2		STX		ADXWK
			ASLB
			ROLA
			ADDB	ADXWK+1
			ADCA	ADXWK
			STAB	ADXWK+1
			STAA	ADXWK
			LDX		ADXWK
			RTS
			ENDIF
			IF	0
*
*	MULTIPLY	AB = AB * (IX,IX+1)
*
MULTIPLY	STAB    W68+1
			STAA    W68
			BEQ		MULTI0
			LDAB	1,X
			LDAA	0,X
			STAB	W66+1
			STAA	W66
MULTI		LDX		#16
			CLRA
			CLRB
ML1			LSR     W68
			ROR     W68+1
			BCC     ML2
			ADDB    W66+1
			ADCA    W66
ML2			ASL     W66+1
			ROL     W66
			DEX
			BNE     ML1
			RTS
*
*	MULTIPLY	AB = B * (IX,IX+1)
*	when AccA==0
*
MULTI0		LDAB	1,X
			STAB	W66+1
			LDAB	0,X
			STAB	W66
			LDX		#8
			CLRB
ML01		LSR     W68+1
			BCC     ML02
			ADDB    W66+1
			ADCA    W66
ML02		ASL     W66+1
			ROL     W66
			DEX
			BNE     ML01
			RTS
			ELSE
*
*	MULTIPLY	AB = (IX,IX+1)*AB
*
MULTIPLY	STX		TDXWK
			LDX		0,X				;   6
			STX		W66				;	4
			STAB	W68+1			;	4
			STAA	W68				;	4
			BEQ		MULTI02			;	4
MULTI		LDX		#16             ;   3
			CLRA					;   2
			CLRB					;   2           CPX #?
			FCB		$8C				;	3			SKIP2
ML01	    ASLB					;   2			loop 34cycle/loop
			ROLA					;   2
			ROL     W68+1			;   6
			ROL     W68				;   6
			BCC     ML02			;   4
			ADDB	W66+1			;   3
			ADCA	W66             ;   3
ML02		DEX						;	4
			BNE     ML01            ;   4
			LDX		TDXWK
			RTS
*
*	MULTIPLY	AB = B * (IX,IX+1)	when AccA==0
*
MULTI02		LDX		#8				;	3		before 3+2+2=7
			CLRA					;	2
			CLRB					;	2
ML201	    ASLB					;	2		loop 24cycle/loop -> 192cycle
			ROLA					;	2
			ROL     W68+1			;	6
			BCC     ML202			;	4
			ADDB	W66+1			;	3
			ADCA	W66             ;	3
ML202		DEX						;	4
			BNE     ML201           ;	4
			LDX		TDXWK
			RTS
			ENDIF
*
*	Division by powers of 2
*			AccAB		Number to be divided
*			SP+2,SP+3	Mask (2^-1)
*			SP+4		Number of shifts
*
DIVPOW2		TSX
			STAA	W66
			BPL		DIVPOW01
			NEGA			; If the dividend is negative, make it positive
			NEGB
			SBCA	#0
DIVPOW01	PSHB
			PSHA
			ANDB	3,X		; mask
			ANDA	2,X		; mask
			STAB	_MOD+1
			STAA	_MOD
			PULA
			PULB
DIVPOW02	ASRA
			RORB
			DEC		4,X
			BNE		DIVPOW02
			TST		W66
			BPL		DIVPOW99
			NEGA			; The dividend was negative, take back to negative number.
			NEGB
			SBCA	#0
DIVPOW99	RTS
			IF	0
*
*	AB <= (SP+4,SP+5)/(SP+2,SP+3)
*
DIVIDE		TSX
			INX
			INX
			LDAA	2,X
			LDAB	3,X
*
*	DIV0	AB <= AB/(IX,IX+1), W4E <= MOD
*
DIV0		CLR     W82+5
			TST     0,X
			BNE     DV1
			TST     1,X
			BNE     DV3
			JMP     ERROR
DV1			BPL     DV3
			INC     W82+5
			NEG     1,X
			BNE     DV2
			DEC     0,X
DV2			COM     0,X
DV3			TSTA
			BPL     DV4
			DEC     W82+5
			BSR     NEGAB
DV4			JSR     DIVPOS
			STAA    W4E
			STAB    W4E+1
			LDAA    W68
			LDAB    W68+1
			TST     W82+5
			BEQ     NEGE
NEGAB		NEGA
			NEGB
			SBCA	#0
NEGE		RTS
*
*	DIVIDE Positive W68 = AB/(X,X+1), AB=MODULO
*
DIVPOS		CLR     W66
DP1			INC     W66
			ASL     1,X
			ROL     0,X
			BCC     DP1
			ROR     0,X
			ROR     1,X
			CLR     W68
			CLR     W68+1
DP2			SUBB    1,X
			SBCA    0,X
			BCC     DP3
			ADDB    1,X
			ADCA    0,X
			CLC
			FCB     $9C
DP3			SEC
			ROL     W68+1
			ROL     W68
			DEC     W66
			BEQ     DIVPE
			LSR     0,X
			ROR     1,X
			BRA     DP2
DIVPE		RTS
			ELSE
*  NEW!
*
*	AB <= (SP+4,SP+5)/(SP+2,SP+3)
*
DIVIDE		TSX
			INX
			INX
			LDAA	2,X
			LDAB	3,X
*
*	DIV0	AB <= AB/(IX,IX+1), W4E <= MOD
*
DIV0		CLR     W82+5
			TST     0,X
			BNE     DV1
			TST     1,X
			BNE     DV3
			JMP     ERROR
DV1			BPL     DV3
			INC     W82+5
			NEG     1,X
			BNE     DV2
			DEC     0,X
DV2			COM     0,X
DV3			TSTA
			BPL     DV4
			DEC     W82+5
			BSR     NEGAB
DV4			JSR     DIVPOS
			STAA    W4E
			STAB    W4E+1
			LDAA    W68
			LDAB    W68+1
			TST     W82+5
			BEQ     NEGE
NEGAB		NEGA
			NEGB
			SBCA	#0
NEGE		RTS
*
*	DIVIDE Positive W68 = AB/(IX,IX+1), AB=MODULO
*
DIVPOS		LDX		0,X				; 6
			STX		W66				; 5
			LDX     #0				; 3
			TST		W66				; 6
			BNE		DP1				; 4		↑24cycle >256の場合
			PSHA					; 4
			LDAA	W66+1			; 3
			STAA	W66				; 4
			CLR		W66+1			; 6
			PULA					; 4
			LDX		#8				; 3		↑24cycle →48cycle <256の場合
DP1			INX						; 4
			ASL     W66+1			; 6
			ROL     W66				; 6
			BCC     DP1				; 4
			ROR     W66
			ROR     W66+1
			CLR     W68
			CLR     W68+1
DP2			SUBB    W66+1
			SBCA    W66
			BCC     DP3
			ADDB    W66+1
			ADCA    W66
			CLC
			FCB     $9C
DP3			SEC
			ROL     W68+1
			ROL     W68
			DEX
			BEQ     DIVPE
			LSR     W66
			ROR     W66+1
			BRA     DP2
DIVPE		RTS
			ENDIF
*
*	RANDOM 1 between AB
*			cf.https://tinyework.flston.com/xorshift-on-hd6301
*
RANDOM		STAB	W6A+1
			STAA	W6A
			LDAB	RND
			LSRB
			LDAB	RND+1
			RORB
			EORB	RND
			STAB	RND
			RORB
			EORB	RND+1
			STAB	RND+1
			TBA
			EORA	RND
			STAA	RND
			LDX		#W6A
			TAB
			CLRA
			JSR		MULTIPLY
			TAB
			CLRA
			ADDB	#1
			ADCA	#0
			RTS
*
*
*
ERROR		LDX		#DIV0ERROR
			JSR		PRINTSTR
			SWI
*
* PRINT HEX sub. AccAB is destroyed
*
* PRINT 4 HEX AccAB
PRHEX4		PSHB
			TAB
			BSR     PR2HEX
			PULB
* PRINT 2 HEX AccB
PRHEX2		TBA
PR2HEX		BSR     PRHEXH
			TAB
			ANDB    #$F
			BRA     PRHEXL
* PRINT 1 HEX AccB high nibble
PRHEXH		LSRB
			LSRB
			LSRB
			LSRB
* PRINT 1 HEX AccB low nibble
PRHEXL		CMPB    #$A
			BMI     PH1
			ADDB    #7
PH1			ADDB	#$30
			PSHA
			TBA
			JSR		ASCOUT
			PULA
			RTS
*
* PRINT AccB of SPACE 
*
PRINTTAB	LDAA	#' '
			TSTB
			BEQ     TABE
TAB1		JSR		ASCOUT
			DECB
			BNE     TAB1
TABE		RTS
*
*	PRINT AccAB decimal left
*
PRINTL		CMPA	#0
			BNE		PRINTL1
			CMPB	#10
			BCS		PR1DEC
PRINTL1		BSR     BDSGN
PRINTSTR	LDAA    0,X
			INX
			TSTA
			BEQ		TABE
	        JSR     ASCOUT
			BRA     PRINTSTR
*
*		1桁の数値(0-9)は計算するまでもない
*
PR1DEC		ADDB	#'0
			TBA
			JMP		ASCOUT
*
*	PRINT (SP+2,SP+3) decimal right AccB
*
PRINTR		STAB	W88			; 指定した桁数
			TSX
			LDAA	2,X
			LDAB	3,X
			BSR		BDSGN
			STX		W64			; 
			LDAB	W88			; 右詰め桁数
			ADDB	W64+1		; 表示する最初の桁のアドレス下位
			SUBB	#W82+6		; バッファ最終桁+1のアドレス下位
			BMI		PR1
			BSR		PRINTTAB
PR1			BRA		PRINTSTR
*
*	符号付きDecimal作成
*	AccAB => (IX)
*
BDSGN		TSTA
			BMI		BDNEG
			BRA		BYNDEC
BDNEG		JSR		NEGAB
			BSR		BYNDEC
			LDAB	#'-'
			DEX
			STAB	0,X
PRE			RTS
OUTNUM		PSHB
			PSHA
			LDAB	#5
			BSR		PRINTR
			RTS
*
*	AccAB to Decimal W82+1..W82+5
*			return IX decimal top (zero blanking)
*
BYNDEC		LDX		#W82+1			; 10進文字保存領域。W82+1〜W82+5
			STX		W6C
			LDX		#CTABLE
BYNDEC0		CLR		W68				; 商0-9
			STX		W6E
BYNDEC2		INC		W68				; 商を+1
			SUBB	1,X
			SBCA	0,X
			BCC		BYNDEC2
			ADDB	1,X				; 引きすぎた
			ADCA	0,X
			PSHA
			LDAA	W68				; 商を文字に変換
			ADDA	#'0'-1			; 1つ多いはずなので引く
			STX		W6E
			LDX		W6C
			STAA	0,X
			INX
			STX		W6C
			PULA
			LDX		W6E
			INX
			INX
			CPX		#CTABLEE
			BNE		BYNDEC0
			ADDB	#'0'			; 余り(AccAB)が最後の1桁になっている
			COMB					; ゼロブランキングのフラグとして使うので反転
			STAB	W82+5
ZEROBLK		LDX		#W82			; 変換終了。ゼロブランキングする
ZB1			INX						; Zero blanking
			LDAB	0,X
			CMPB	#'0'
			BEQ		ZB1
			COM		W82+5
ZB2			RTS
CTABLE		FDB     10000
			FDB     1000
			FDB     100
			FDB     10
;			FDB		1
CTABLEE		EQU		*
			ENDIF
*
BEL			EQU		$07
BS			EQU		$08
CR			EQU		$0D
DEL			EQU		$7F
*
PRINTCR		LDAA	#CR
			JMP     ASCOUT
*
INPLIN		LDX     #LINBUF+1
IL2			DEX
IL3			LDAA	#'_'
			JSR		ASCIN
			CMPA	#$D
			BNE		IL30
			CLR		0,X
			JMP		ASCOUT
IL30		CMPA	#DEL
			BEQ		IL31
			CMPA	#BS
			BNE		IL32
			LDAA	#DEL
IL31		JSR		ASCOUT
			CPX		#LINBUF
			BNE		IL2
IL311		LDAA	#BEL
			JSR		ASCOUT
			BRA		IL3
IL32		CMPA	#'-'				; 先頭だけは '-' を受け付ける
			BNE		IL323
			CPX		#LINBUF
			BNE		IL3
			BRA		IL33
IL323		CMPA	#'0'				; 入力できるのは数字だけ
			BCS		IL311
			CMPA	#'9'+1
			BCC		IL311
			CPX		#LINBUF+LBUFSIZ
			BEQ		IL311
IL33		JSR		ASCOUT
			STAA	0,X
			INX
			BRA     IL3
*
INPUT		BSR     INPLIN			; 1行読んでから
DECBIN		LDX		#LINBUF-1		; バッファの文字列を数値に
			LDAB	1,X
			CMPB	#'-'
			BNE		DB0
			INX
			BSR		DB0
			NEGA
			NEGB
			SBCA	#0
			RTS
DB0			CLRB
			CLRA
DB1			INX
			STAB	DBWK+1			; 今までに読み込んだ数値を保存
			STAA	DBWK
			LDAB	0,X
			SUBB	#'0
			BCS		DBE
			CMPB	#10
			BCC		DBE
			STAB	0,X				; バッファを壊すがどうせ使わない
			LDAB	DBWK+1
			ASLB					; 今までの数値を10倍
			ROLA
			ASLB
			ROLA
			ADDB	DBWK+1
			ADCA	DBWK
			ASLB
			ROLA
			ADDB	0,X				; 新しい桁を足す
			ADCA	#0
			BRA		DB1
DBE			LDAB	DBWK+1
			RTS
*
INIT_MUSIC	CLR $00C4
			CLR $00C5
			CLR $00C7
			CLR $00CA
			CLR $00CB
			DEC $00CB
			RTS
*
KBIN_SUB	JSR	KBIN
			BCS	NOKEYIN
			TAB
			FCB	$81					; skip 1byte
NOKEYIN		CLRB
			CLRA
			RTS
*
DIV0ERROR	FCB	$0D
			FCC	'DIVIDE BY 0 ERROR'
			FCB	$0D,$07,$00
*
*	Workarea
*
			IF		0
ADXWK		EQU		$0061
			ENDIF
TDXWK		EQU		$0063
IXSAVE		RMB		2
DBWK		RMB		2
W4E			RMB		2
_MOD		EQU		W4E
W52			EQU		$00E0
RND			EQU		W52
W64			RMB		2
;W66			RMB		2
;W68			RMB		2
W66			EQU		$005D
W68			EQU		$006F
W6A			RMB		2
W6C			RMB		2
W6E			RMB		2
W70			RMB		2
W82			RMB		6
			FCB		0
W88			RMB		1
PRGEND		EQU		*
