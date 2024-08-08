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
*
*	MULTIPLY	AB = AB * (IX,IX+1)
*
MULTIPLY	STAB    W68+1
			STAA    W68
			BEQ		MULTI0
			LDAB    #16
			STAB    W66
			CLRA
			CLRB
ML1			LSR     W68
			ROR     W68+1
			BCC     ML2
			ADDB    1,X
			ADCA    0,X
ML2			ASL     1,X
			ROL     0,X
			DEC     W66
			BNE     ML1
			RTS
*
*	MULTIPLY	AB = B * (IX,IX+1)
*	when AccA==0
*
MULTI0		LDAB    #8
			STAB    W66
			CLRA
			CLRB
ML01		LSR     W68+1
			BCC     ML02
			ADDB    1,X
			ADCA    0,X
ML02		ASL     1,X
			ROL     0,X
			DEC     W66
			BNE     ML01
			RTS
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
*	DIVIDE Positive AB = AB/(X,X+1), no MODULO
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
*
*	RANDOM 1 between AB
*
RANDOM		STAA    W6A
			STAB    W6A+1
			LDX     #RND
			LDAA    #$3D
			LDAB    #$09
			JSR     MULTIPLY
			BSR     ADD1
			STAA    RND
			STAB    RND+1
			LDX     #W6A
			TAB
			CLRA
			JSR     MULTIPLY
			TAB
			CLRA
ADD1		ADDB    #1
			ADCA    #0
			RTS
ERROR		SWI
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
PRINTL		BSR     BDSGN
PRINTSTR	LDAA    0,X
			INX
			TSTA
			BEQ		TABE
	        JSR     ASCOUT
			BRA     PRINTSTR
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
*** bynary(AccAB) to decimal(IX) ***
*
BYNDEC		LDX		#W82+1
			STX		W6C
			LDX		#CTABLE
BDL			STX		W6E
			LDX		0,X
			STX		W70
			LDX		#W70
			JSR		DIVPOS
			PSHA
			LDX		W6C
			LDAA	W68+1
			ADDA	#$30
			STAA	0,X
			INX
			STX		W6C
			LDX		W6E
			PULA
			INX
			INX
			TST		1,X
			BNE		BDL
			LDX		#W82
*
*** ZERO BLANKING ***
*
ZROBLK		COM		5,X
ZB1			INX
			LDAB	0,X
			CMPB	#'0'
			BEQ		ZB1
			COM		W82+5
ZB2			RTS
*
CTABLE		FDB     10000
			FDB     1000
			FDB     100
			FDB     10
			FDB     1
			FDB     0
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
*	Workarea
*
ADXWK		RMB		2
DBWK		RMB		2
W4E			RMB		2
_MOD		EQU		W4E
W52			RMB		2
RND			EQU		W52
W64			RMB		2
W66			RMB		2
W68			RMB		2
W6A			RMB		2
W6C			RMB		2
W6E			RMB		2
W70			RMB		2
W82			RMB		6
			FCB		0
W88			RMB		1
PRGEND		EQU		*
