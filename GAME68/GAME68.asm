* TITLE GAMEIII
***************************
*  GENERAL ALGOLITHMIC    *
*   MICRO EXPRESSIONS     *
*    (VERSION III-2)	  *
*-------------------------*
*   BY H.OHNISHI	  *
*	1978/5/7          *
***************************
*   port to BASIC MASTER  *
*	by ZUKERAN, shin  *
*	(TEW SOFT,@zu2)   *
***************************
;STACK1	EQU	$FF
	ORG	$E0
SP2	RMB	2
W5A	RMB	2
W66	RMB	2
W68	RMB	2
W6A	RMB	2
W70	RMB	2
W72	RMB	2
W76	RMB	2
ADRVWK	RMB	2		; 変数アドレス変換用
;
;SP2	RMB	2
;	ORG	$0072
	ORG	$0400
VAR0	EQU	(*/256)*256
VARTOP	RMB	2	    ; @
	RMB	26*2	    ; A-Z
	RMB	2	    ; [
W3C	RMB	2	    ; \
	RMB	2	    ; ]
	RMB	2	    ; ^
W42	RMB	2	    ; _
	RMB	2	    ; ' '
W46	RMB	2	    ; !
	RMB	2	    ; "
W4A	RMB	2	    ; #
	RMB	2	    ; $
W4E	RMB	2	    ; %
W50	RMB	2	    ; &
PGE	EQU	W50
W52	RMB	2	    ; '
RND	EQU	W52
	RMB	2	    ; (
	RMB	2	    ; )
W58	RMB	2	    ; *
PGL	EQU	W58
oW5A	RMB	2	    ; +
	RMB	2	    ; ,
	RMB	2	    ; -
	RMB	2	    ; .
	RMB	2	    ; /
W64	RMB	2	    ; 0
oW66	RMB	2	    ; 1
oW68	RMB	2	    ; 2
oW6A	RMB	2	    ; 3
W6C	RMB	2	    ; 4
W6E	RMB	2	    ; 5
oW70	RMB	2	    ; 6
oW72	RMB	2	    ; 7
W74	RMB	2	    ; 8
oW76	RMB	2	    ; 9
	RMB	2	    ; :
	RMB	2	    ; ;
	RMB	2	    ; <
W7E	RMB	2	    ; =
PGB	EQU	W7E
	RMB	2	    ; >
W82	RMB	6	    ; ?	    print buffer
W88	RMB	1
isBREAK	RMB	1
FLOAD	RMB	1
FSAVE	RMB	1
LSWK	RMB	2		; LOAD/SAVE IX save area
LSWK2	RMB	2		; LOAD/SAVE IX save area2
LSWK3	RMB	2		; LOAD/SAVE IX save area3
STACK2	EQU	$04FF
LINBUF	EQU	$0500
LBUFSIZ	EQU	255-1
NBUF	EQU	$0600
	ORG	$0800
IOBUF	RMB	256		; SAVE/LOAD BUF
*
TXTBEG	EQU	&TXTBEG
;RAMEND	EQU	$5FFF
*
* BM Monitor subroutine
*
USRNMI	EQU	$0000
USRIRQ	EQU	$0002
BREAKV	EQU	$0004
TIMERV	EQU	$0006
RAMEND	EQU	$0008	    ; BM Monitor workarea
TIME1	EQU	$000A
TIME2	EQU	$000B
TIME3	EQU	$000C
CURX    EQU     $000F
CURY    EQU     $0010
ASCIN	EQU	$0028
ASCOUT	EQU	$002B
BYTIN	EQU	$002E
BYTOUT	EQU	$0031
RECTOP	EQU	$003B
LDTOP	EQU	$003B
MSTTOP	EQU	$003B
RECEND	EQU	$003D
LDEND	EQU	$003D
MSTEND	EQU	$003D
CPYTOP	EQU	$003F
CPYEND	EQU	$0041
FNAME	EQU	$0043
MONITOR	EQU	$F000
MUSIC	EQU	$F00C
MOVBLK	EQU	$F009
KBIN	EQU	$F00F
CHRGET	EQU	$F012
CHROUT	EQU	$F015
NMISET	EQU	$FFE6	    ; inhibit NMI
NMICLR	EQU	$FFE9	    ; allow NMI
CLRTV	EQU	$FFEF
CURPOS	EQU	$FFF5
;OUTBBB	EQU	$700
;INPBBB	EQU	$799
;TSTBRK	EQU	$7BD
*
BEL	EQU	$07
BS	EQU	$08
CR	EQU	$0D
RUB	EQU	BS
DEL	EQU	$7F
*
	ORG	&PRGBEG
*** INTERPRETER START ***
*
*(SET USER'S INIT.ROUTINE)
COLDST	JMP	CSTRT
*
HOTST	BRA	HSTRT
NMICLI	PSHA
	JSR	NMICLR
	CLI
	PULA
	RTS
NMISEI	PSHA
	SEI
	JSR	NMISET
	PULA
	RTS
BREAK2	TPA
	STAA	isBREAK
	RTI
OUTBBB	PSHA
	TBA
	JSR	ASCOUT
	TST	FSAVE
	BEQ	OUTBBB1
	JSR	SHOOK
OUTBBB1	TAB
	PULA
	RTS
CSTRT	LDS	RAMEND
	LDX	#TXTBEG
	STX	PGB
	STX	PGE
	LDAA	#$FF
	STAA	0,X
	CLR	FLOAD
	CLR	FSAVE
	LDX	#STARTM
	JSR	OUTSTR
HSTRT	LDB	RAMEND+1
	STAB	PGL+1
	LDB	RAMEND
	DECB
	STAB	PGL
	CLR	isBREAK
	LDX	#BREAK2
	STX	BREAKV
	LDX	#VARTOP
	STX	ADRVWK
	CLR	FLOAD
	CLR	FSAVE
	JSR	INIT_MUSIC
*
*** MAIN ROUTINE ***
*
MAIN	LDS	RAMEND
	CLR	isBREAK
	LDX	#STACK2
	STX	SP2
	LDX	#READY
	JSR	OUTSTR
	BSR	NMICLI	    ; enable NMI/IRQ
PMODE	LDS	RAMEND
	JSR	INPCOM
	BCS	TSTLST
	STX	W42
	JSR	EXCSTD
	BEQ	MAIN
NEWL	JSR	SRCLIN
EXCL	BMI	MAIN	    ; プログラム末に来たので終了
	STX	W42
THIS	TST	isBREAK
	BEQ	EXCL2
	CLR	FLOAD
	CLR	FSAVE
	LDX	#BREAKM
	JSR	OUTSTR
	BRA	MAIN
EXCL2	INX		    ; 行番号Skip
	INX
	LDAA	0,X
	CMPA	#' '	    ; コメント?
	BNE	NXTL	    ;   Yes→次の行へ
	JSR	EXCSTP
	BNE	JUMP
	INX
	TST	0,X
	BRA	EXCL
NXTL	INX
	LDAA	0,X
	BNE	NXTL
	INX
	STX	W42
	LDAA	0,X
	BRA	EXCL
JUMP	LDX	W42
	STX	W72
	BSR	SRCH
	BMI	MAIN
	CPX	W72
	BEQ	NEWL
	BRA	THIS
*
*** LIST/EDIT ***
*
TSTLST	STX	W74
	STAA	W4A
	STAB	W4A+1
	LDAB	0,X
	CMPB	#'/'
	BNE	EDITER
;	JMP	LEDIT
*
*** LIST PROGRAM ***
*
LIST	BSR	SRCLIN
LLP	JSR	CRLF
LLP1	TST	0,X
	BMI	LLP2
	BSR	OUTLIN
	TST	isBREAK
	BNE	LLP2
	BRA	LLP
LLP2	TST	FSAVE
	BEQ	LLP4
	CLR A			    ; SAVE末に00出力
	JSR	SHOOK
	LDX	$003B		    ; SAVE残りあり？
	CPX	$003D
	BEQ	LLP3
	JSR	SHSUB
LLP3	CLR	FSAVE
LLP4	JMP	MAIN
*
*** LIST A LINE ***
*
OUTLIN	STX	W42
	LDAA	0,X
	LDAB	1,X
	TST	FSAVE
	BNE	OUTLIN2
	JSR	OUTNUM
	BRA	OUTLIN3
OUTLIN2	JSR	BYNDEC
	BSR	OUTLIN4
OUTLIN3	LDX	W42
	INX
	INX
OUTLIN4	JMP	OUTSTR
*
*** SEARCH LINE END ***
*
SRCZRO	LDX	W42
SZ0	INX
SZ1	INX
	TST	0,X
	BNE	SZ1
	INX
	RTS
*
*** SEARCH THE LINE ***
*
*   W4Aに格納されている行番号を探す
*
SRCLIN	LDX	PGB	; 先頭から探す
SL	STX	W42
SRCH	LDAA	1,X	; 現在のIXから探す
	SUBA	W4A+1
	LDAA	0,X
	BMI	SLE
	SBCA	W4A
	BCC	FOUND
NXTLIN	BSR	SRCZRO
	BRA	SL
FOUND	CLRA
SLE	RTS
*
*** EDITER ***
*
EDITER	LDX	W4A
	BEQ	LIST
	BMI	EREND
	LDX	PGE
	LDAA	0,X
	INCA
	BNE	EREND
	JSR	NMISEI	    ; inhibit NMI/SEI
	BSR	SRCLIN
	BMI	LENGTH
	LDX	0,X
	CPX	W4A
	BNE	LENGTH
DELETE	BSR	SRCZRO
	LDS	W42
D0	LDAA	0,X
	PSHA
	INS
	CPX	PGE
	BEQ	D1
	INX
	INS
	BRA	D0
D1	STS	PGE
LENGTH	LDX	W74
	LDAB	#3
	TST	0,X
	BEQ	EDEND
L1	INCB
	INX
	TST	0,X
	BNE	L1
	CLRA
	ADDB	PGE+1
	ADCA	PGE
	STAA	W3C
	STAB	W3C+1
	SUBB	PGL+1
	SBCA	PGL
	BCS	MSPACE
EREND	JMP	MAIN
MSPACE	LDX	PGE
	LDS	W3C
	STS	PGE
	INX
MS1	DEX
	LDAB	0,X
	PSHB
	CPX	W42
	BNE	MS1
INSERT	LDS	W4A
	STS	0,X
	LDS	W74
	DES
IS1	INX
	PULB
	STAB	1,X
	BNE	IS1
EDEND	LDS	RAMEND
;	LDX	PGE
;	CLR	0,X
;	DEC	0,X
	JSR	NMICLI	    ; enable NMI/IRQ
	JMP	PMODE
*
*** BYNARY TO DECIMAL ***
*
BYNDEC	LDX	#W82+1
	STX	W6C
	LDX	#CTABLE
BDL	STX	W6E
	LDX	0,X
	STX	W70
	LDX	#W70
	JSR	DIVPOS
	PSHA
	LDX	W6C
	LDAA	W68+1
	ADDA	#$30
	STAA	0,X
	INX
	STX	W6C
	LDX	W6E
	PULA
	INX
	INX
	TST	1,X
	BNE	BDL
	LDX	#W82
*
*** ZERO BLANKING ***
*
ZROBLK	COM	5,X
ZB1	INX
	LDAB	0,X
	CMPB	#'0'
	BEQ	ZB1
	COM	W82+5
ZB2	RTS
*
*** OUTPUT CHR STRING ***
*
OUTSTR	CLRA
OST1	STAA	W88
O2	LDAB	0,X
	INX
	CMPB	W88
	BEQ	ZB2
	JSR	OUTB
	BRA	O2
*
*** BREAK ***
*
;BREAK	TST	isBREAK
;	BEQ	ZB2
;	JMP	MAIN
;	BSR	INPB
;	CMPB	#3
;	BEQ	EREND
;	CMPB	#1
;	BNE	ZB2
;INPB	JMP	INPBBB
INPB	PSHA
	TST	FLOAD
	BEQ	INPB1
	JSR	LHOOK
	BRA	INPB0
INPB1	LDAA	#'_'
	JSR	ASCIN
INPB0	TAB
	PULA
	RTS
*
*** EXECUTE STATEMENTS ***
*
STM2	CMPA	#' '
	BEQ	EXCSTP
	CLRB
	STAB	W4A
	STAB	W4A+1
NOPSTM	LDAB	1,X
	BITB	#$DF	    ; 2文字目が$20 or $00 ?
	BNE	LITSTR	    ; NO
RETURN	CMPA	#']'	    ; 1文字のステートメントは ']' か '@' のはず
	BNE	DO
	JSR	PULPNT
	BRA	STM3
DO	CMPA	#'@'
	BNE	LITSTR
	INX
	STX	W46
	CLRA
	CLRB
	JSR	PSHVP
	BRA	STM3
LITSTR	CMPA	#'"'
	BNE	LSCRLF
	INX
	BSR	OST1
	BRA	EXCSTD
LSCRLF	CMPA	#'/'
	BNE	LOADSAVE
	JSR	CRLF
	BRA	EXCSTP
*
ARTSTM	STX	W72
	JSR	SRCEQU
	JSR	COMAND
;	BSR	BREAK
	LDX	W4A
	BNE	STM4
STM3	LDX	W46
	FCB	$9C
EXCSTP	INX
EXCSTD	LDAA	0,X	    ; 行末か?
	BNE	STM2
STM4	RTS
*
LOADSAVE    CMPA    #'['
	BNE	ARTSTM
	INX
	LDAA	0,X
	CMPA	#'S'
	BNE	LS1
	JSR	SAVE
LS0	JMP	MAIN
LS1	CMPA	#'L'
	BNE	LS2
	JSR	LOAD
	BRA	LS0
LS2	CMPA	#'A'
	BNE	LS3
	JSR	APPEND
	BRA	LS0
LS3	CMPA	#'='
	BNE	LSERR
	INX
	LDAA	0,X
	CMPA	#'0'		; GOTO MONITOR
	BNE	LS4
	JSR	MONITOR
	JMP	MAIN
LS4	CMPA	#'1'		; MUSIC
	BNE	LSERR
LS5	INX
	LDAA	0,X
	BEQ	STM4
	CMPA	#' '
	BEQ	LS5
	JSR	MUSIC
	LDAA	0,X
	CMPA	#'%'
	BNE	EXCSTD
	BRA	EXCSTP
*
LSERR	LDAA	#'['
	BRA	SE01
*
*** PUSH VER & POINTERS ***
*
PSHVP	STAA	W6A
	STAB	W6A+1
	JSR	PSHPNT
	LDX	#W6A
	JMP	PSHX
*
*** SECOND ARTH-EXP ***
*
SECAE	PSHB
	INX
	BSR	ARTEXP
	BSR	PSHVP
	PULB
	PULA
	RTS
*
*** SEARCH "=" MARK ***
*
SE1	LDAA	#'='
SE01	JMP	ERROR
SRCEQU	LDAA	0,X
	INX
	BITA	#$DF		; $00 or $20?
	BEQ	SE1
	CMPA	#'='
	BNE	SRCEQU
*
*** ARITHMETIC EXPRESSION ***
*
ARTEXP	BSR	TERM
AE1	PSHA
	LDAA	0,X
	BITA	#$DF
	BEQ	AE2
	CMPA	#')'
	BEQ	AE3
	CMPA	#','
	BEQ	SECAE
	PULA
	JSR	BYNOPE
	LDX	W5A
	BRA	AE1
AE2	STX	W46
	FCB	$9C
AE3	INX
	PULA
	RTS
*
*** TERM(1) ***
*
TERM	JMP	TERM2
TERM3	CMPB	#'?'
	BNE	TM1
	STX	W76
	JSR	INPLIN2
	BSR	ARTEXP
	LDX	W76
	INX
	RTS
TM1	CMPB	#'('
	BNE	GETVAR
	INX
AEB	BRA	ARTEXP
GETVAR	BSR	VNAME
	CMPA	#':'
	BEQ	GET1B
GET2B	BSR	ADR2B
	LDAA	0,X
	LDAB	1,X
	BRA	GB1
GET1B	BSR	ADR1B
	CLRA
	LDAB	0,X
GB1	LDX	W70
	RTS
*
*** VARIABLE ADDRESS ***
*
ADR1B	BSR	ADRIND
AD0	ADDB	W76+1
	ADCA	W76
	BRA	AV1
ADR2B	CMPA	#'('
	BNE	ADRVAR
	BSR	ADRIND
	ASLB
	ROLA
	BRA	AD0
ADRIND	BSR	ADRVAR
	LDX	0,X
	STX	W76
	LDX	W70
	INX
	BRA	AEB
VNAME	LDAB	0,X
VN1	INX
	LDAA	0,X
	CMPA	#'A'
	BPL	VN1
	RTS
ADRVAR	ANDB	#$3F
;	ADDB	#(VARTOP-VAR0)/2
	ASLB
	STAB	ADRVWK+1
	STX	W70
	LDX	ADRVWK
	RTS
AV1	STX	W70
	JMP	TSTZR0
*
*** BYNARY OPERATION ***
*
BYNOPE	PSHA
	PSHB
	LDAA	0,X
	PSHA
	LDAB	1,X
	PSHB
	SUBB	#'='
	BEQ	TCHR
	DECB
	BNE	OCHR
TCHR	INX
OCHR	INX
	BSR	TERM
	STAA	W6A
	STAB	W6A+1
	STX	W5A
	LDX	#W6A
	PULB
	PULA
* EQU *
	CMPA	#'='
	BNE	BO1
	PULB
	PULA
EQUAL	BSR	SUB
EQ1	BNE	FALSE
	TSTB
	BNE	FALSE
TRUE	CLRA
	LDAB	#1
	RTS
* NEQ & LT & LE *
BO1	CMPA	#'<'
	BNE	BO2
	CMPB	#'='
	BEQ	LE
	CMPB	#'>'
	PULB
	BNE	LT
	PULA
	BSR	EQUAL
	BRA	NTS
LE	PULB
	PULA
	BSR	GRATER
	BRA	NTS
LT	PULA
LESS	BSR	SUB
	BLT	TRUE
FALSE	CLRA
	CLRB
	RTS
* GT & GE *
BO2	CMPA	#'>'
	BNE	BO3
	CMPB	#'='
	PULB
	BNE	GT
	PULA
	BSR	LESS
	BRA	NTS
GT	PULA
GRATER	BSR	SUB
	BLT	FALSE
	BSR	EQ1
NTS	EORB	#1
	RTS
BO3	PULB
* ADDITION *
	CMPA	#'+'
	BNE	BO4
	PULA
ADD	ADDB	1,X
	ADCA	0,X
	RTS
* SUBTRACTION *
BO4	CMPA	#'-'
	BNE	BO5
	PULA
SUB	SUBB	1,X
	SBCA	0,X
	RTS
* MULTIPLICATION *
BO5	CMPA	#'*'
	BNE	BO6
	PULA
MLTPLY	STAA	W68
	STAB	W68+1
	LDAB	#16
	STAB	W66
	CLRA
	CLRB
ML1	LSR	W68
	ROR	W68+1
	BCC	ML2
	BSR	ADD
ML2	ASL	1,X
	ROL	0,X
	DEC	W66
	BNE	ML1
	RTS
BO6	JMP	BO8
*
*** CONSTANT FOR 2-10 ***
*
CTABLE	FDB	10000
	FDB	1000
	FDB	100
	FDB	10
	FDB	1
	IF	W66>=$100
	FDB	0	    ; 次のCLR命令がEXTENDで7F0066であることを期待していた
	ENDIF
*
*** POSITIVE DIVISION ***
*
DIVPOS	CLR	W66
DP1	INC	W66
	ASL	1,X
	ROL	0,X
	BCC	DP1
	ROR	0,X
	ROR	1,X
	CLR	W68
	CLR	W68+1
DP2	BSR	SUB
	BCC	DP3
	BSR	ADD
	CLC
	FCB	$9C
DP3	SEC
	ROL	W68+1
	ROL	W68
	DEC	W66
	BEQ	TDE
	LSR	0,X
	ROR	1,X
	BRA	DP2
	IF	0
*
*** TEST DECIMAL NO. ***
*
TSTDEC	LDAB	0,X
	CMPB	#'0'
	BCS	TD1
	CMPB	#'9'+1
	RTS
	ENDIF
TD1	CLC
	RTS
TD20	PULB
TD2	SEC
TDE	RTS
*
*** INPUT COMMAND ***
*
INPCOM	BSR	IL1
*
*** DECIMAL TO BYNARY ***
*
*   数字以外ならC=0でreturn
*
DECCNS	LDAB	0,X
	CMPB	#'0'
	BCS	TD1
	CMPB	#'9'+1
	BCC	TDE
DECBYN	CLRA
	CLRB
DB1	ADDB	0,X
	ADCA	#0
	SUBB	#'0'
	SBCA	#0
	STAA	W66
	STAB	W66+1
	INX
	PSHB
	LDAB	0,X
	CMPB	#'0'
	BCS	TD20
	CMPB	#'9'+1
	BCC	TD20
	PULB
	ASLB
	ROLA
	ASLB
	ROLA
	ADDB	W66+1
	ADCA	W66
	ASLB
	ROLA
	BRA	DB1
*
*** INPUT TO LINE BUFFER **
*
;IL1	LDAB	#CR
;	BSR	OUTBB
INPLIN	LDAB	#':'
IL1	EQU	INPLIN
	BSR	OUTBB
INPLIN2	LDX	#LINBUF+1
IL2	DEX
IL3	JSR	INPB
	CMPB	#$D
	BEQ	IL34
	CMPB	#DEL
	BEQ	IL31
	CMPB	#RUB
	BNE	IL32
IL30	LDAB	#DEL
IL31	BSR	OUTBB
	CPX	#LINBUF
	BNE	IL2
IL311	LDAB	#BEL
	BSR	OUTBB
	BRA	IL3
IL32	CMPB	#$10
	BCS	IL3
	CPX	#LINBUF+LBUFSIZ
	BEQ	IL311
IL33	BSR	OUTBB
	STAB	0,X
	INX
	BRA	IL3
IL34	CLR	0,X
IL4	LDX	#LINBUF
*
*** CR/LF ***
*
CRLF	LDAB	#$D
;	BSR	OUTBB
;LF	LDAB	#$A
OUTBB	JMP	OUTB
*
**** COMMANDS ****
*
COMAND	PSHB
	LDX	W72
	LDAB	0,X
* TAB *
	CMPB	#'.'
	BNE	CM1
	PULB
	TBA
TAB1	TSTA
	BEQ	IFE
	LDAB	#' '
	BSR	OUTBB
	DECA
	BRA	TAB1
* IF *
CM1	CMPB	#';'
	BNE	CM3
	PULB
	BSR	TSTZR0
	BEQ	ELSE
IFE	RTS
ELSE	LDS	RAMEND
	LDX	W42
	JMP	NXTL
*
PULPNT	LDX	#W46		; STATEMENT END
	BSR	PULX
	LDX	#W42		; LINE TOP
PULX	BSR	PULAB
STORE	STAA	0,X
	STAB	1,X
	RTS
* NEXT/UNTIL *
CM3	CMPB	#'@'
	BNE	CM4
	PSHA
	LDAB	2,X
	JSR	ADRVAR
	PULA
	PULB
	BSR	STORE
	BSR	PULAB
	JSR	LESS
	BNE	ADJSP2
MULOOP	BSR	PULPNT		; NEXT
	LDX	SP2
	DEX
	DEX
	DEX
	DEX
	DEX
	DEX
	STX	SP2
	RTS
ADJSP2	STX	W68		; UNTIL
	LDX	SP2
	INX
	INX
	BRA	PULAB1
PULAB	STX	W68
	LDX	SP2
PULAB1	LDAA	0,X
	LDAB	1,X
	INX
	INX
	STX	SP2
	LDX	W68
	RTS
* NEW *
CM4	CMPB	#'&'
	BNE	CM5
	LDAB	1,X
	CMPB	#'='
	BNE	CM5
	PULB
	BSR	TSTZR0
	BNE	NEWE
	LDX	PGB
	STX	PGE
	DECA
	STAA	0,X
NEWE	RTS
TSTZR0	STAA	W6C		; X<-AB
	STAB	W6C+1
	LDX	W6C
TSTZRE	RTS
* GOSUB *
CM5	CMPB	#'!'
	BNE	CM6
	PULB
	STAA	W4A	    ; 行番号をサーチ用Workに入れる
	STAB	W4A+1
	LDX	W4A
	BEQ	TSTZRE
PSHPNT	LDX	#W42	    ; LINE TOP
	BSR	PSHX
	LDX	#W46	    ; STATEMENT END
PSHX	LDAA	0,X
	LDAB	1,X
	STX	W6C
	LDX	SP2
	DEX
	DEX
	STAA	0,X
	STAB	1,X
	STX	SP2
	LDX	W6C
	RTS
* PRINT ASCII CHR *
CM6	CMPB	#'$'
	BNE	CM7
	PULB
OUTB	JMP	OUTBBB
* PRINT DECIMAL RIGHT *
CM7	CMPB	#'?'
	BNE	CM8
	LDAB	1,X
	CMPB	#'('
	BEQ	PRINTR
	JMP	QMARK
PRINTR	PSHA
	INX
	JSR	TERM2
PR2	STAB	W88
	PULA
	PULB
	BSR	BDSGN
	STX	W64
	LDAA	W88
	ADDA	W64+1
	SUBA	#W88
	BMI	PR1
	JSR	TAB1
PR1	JMP	OUTSTR
BDSGN	TSTA
	BMI	BDNEG
BDJ	JMP	BYNDEC
BDNEG	JSR	NEGAB
	BSR	BDJ
	LDAB	#'-'
	DEX
	STAB	0,X
PRE	RTS
OUTNUM	PSHB
	PSHA
	LDAB	#5
	BRA	PR2
* PG ADR CHANGE *
CM8	CMPB	#'='
	BNE	CM9
	PULB
	STAA	PGB
	STAB	PGB+1
	LDX	#$FFFF
	STX	W4A
	JSR	SRCLIN
	STX	PGE
	JMP	MAIN
* GO USER SUB *
CM9	CMPB	#'>'
	BNE	BMCM
	PULB
	JSR	TSTZR0
	BEQ	PRE
	JSR	0,X
	PSHB
* LET *
STAVAR	PSHA
	LDX	W72
	JSR	VNAME
	CMPA	#':'
	BNE	STA2B
STA1B	JSR	ADR1B
	PULA
	PULB
	STAB	0,X
	RTS
STA2B	JSR	ADR2B
	PULA
	PULB
	JMP	STORE
*
BMCM	CMPB	#$5C		; \=expr    TIME
	BNE	BMCM1
	PULB
	SEI
	STAA	TIME1
	STAB	TIME2
	CLR	TIME3
	CLI
	RTS
BMCM1	CMPB	#'^'		; ^=expr	CURXY
	BNE	STAVAR
	PULB
	STAA	CURX
	STAB	CURY
	RTS
*
*** BINARY OPERATION(2) ***
*
*DIVISION *
BO8	CMPA	#'/'
	BNE	BAND
	CLR	W82+5
	TST	0,X
	BNE	DV1
	TST	1,X
	BNE	DV3
	BRA	ERROR
DV1	BPL	DV3
	INC	W82+5
	NEG	1,X
	BNE	DV2
	DEC	0,X
DV2	COM	0,X
DV3	PULA
	TSTA
	BPL	DV4
	DEC	W82+5
	BSR	NEGAB
DV4	JSR	DIVPOS
	STAA	W4E
	STAB	W4E+1
	LDAA	W68
	LDAB	W68+1
	TST	W82+5
	BEQ	NEGE
NEGAB	NEGB
	BNE	NG1
	DECA
NG1	COMA
NEGE	RTS
*
*	AND/OR/XOR拡張
*
BAND	CMPA	#'&'
	BNE	BOR
	PULA
	ANDB	1,X
	ANDA	0,X
	RTS
BOR	CMPA	#'.'
	BNE	BXOR
	PULA
	ORB	1,X
	ORA	0,X
	RTS
BXOR	CMPA	#'!'
	BNE	ERROR
	PULA
	EORB	1,X
	EORA	0,X
	RTS
*
*** ERROR ***
*
ERROR	JSR	CRLF
	LDAB	#'?'
	JSR	OUTB
	TAB
	JSR	OUTB
	LDAB	#' '
	JSR	OUTB
	LDX	W42
	CPX	IL4+1
	BNE	ER1
	JSR	OUTSTR
	BRA	ERE
ER1	JSR	OUTLIN
ERE	JMP	MAIN
*
*** TERM(2) ***
*
TERM2	JSR	DECCNS
	BCS	ABS1
* NEGATIVE *
	CMPB	#'-'
	BNE	TM3
	INX
	BSR	TERM2
NEG1	BRA	NEGAB
* ABSOLUTE VALUE *
TM3	CMPB	#'+'
	BNE	TM4
	INX
	BSR	TERM2
	TSTA
	BMI	NEG1
ABS1	RTS
* NOT *
TM4	CMPB	#'#'
	BNE	TM5
	INX
	BSR	TERM2
	TSTA
	JMP	EQ1
* MODULO *
TM5	CMPB	#'%'
	BNE	TM6
	INX
	BSR	TERM2
	LDAA	W4E
	LDAB	W4E+1
	RTS
* RANDOM NO. *
TM6	CMPB	#'''
	BNE	TM7
	INX
	BSR	TERM2
	STAA	W6A
	STAB	W6A+1
	STX	W70
	LDX	#RND
	LDAA	#$3D
	LDAB	#$09
	JSR	MLTPLY
	BSR	ADD1
	STAA	RND
	STAB	RND+1
	LDX	#W6A
	TAB
	CLRA
	JSR	MLTPLY
	TAB
	CLRA
	LDX	W70
ADD1	ADDB	#1
	ADCA	#0
	RTS
* ASCII CONSTANT *
TM7	CMPB	#'"'
	BNE	TM8
	CLRA
	LDAB	1,X
	INX
	INX
	INX
	RTS
*
TM8	CMPB	#'$'
	BEQ	DOLLAR
	BRA	BMTM
*
*** PRINT/PRINT-HEX ***
*
QMARK	CMPB	#'?'
	BEQ	PRHEX
	CMPB	#'$'
	BEQ	PRHEX2
* PRINT LEFT *
PRINTL	PULB
	JSR	BDSGN
	JMP	OUTSTR
* PRINT 4 HEX *
PRHEX	TAB
	BSR	PR2HEX
* PRINT 2 HEX *
PRHEX2	PULB
	TBA
PR2HEX	BSR	PRHEXH
	TAB
	ANDB	#$F
	BRA	PRHEXL
PRHEXH	LSRB
	LSRB
	LSRB
	LSRB
PRHEXL	CMPB	#$A
	BMI	PH1
	ADDB	#7
PH1	ADDB	#$30
OB1	JMP	OUTB
*
*** "$" MARK IN A.E. ***
*
DOLLAR	CLRA
	BSR	TSTHEX
	BCS	HEXBYN
* ASCII INOUT *
IPB	JMP	INPB
* HEX CONSTANT *
HEXBYN	PSHB
	BSR	TSTHEX
	STAB	W66
	PULB
	BCC	YESHEX
	ASLB
	ROLA
	ASLB
	ROLA
	ASLB
	ROLA
	ASLB
	ROLA
	ADDB	W66
	BRA	HEXBYN
TSTHEX	INX
	LDAB	0,X
	SUBB	#$30
	BCS	NOTHEX
	CMPB	#$A
	BCS	YESHEX
	SUBB	#$7
	CMPB	#$A
	BCS	NOTHEX
	CMPB	#$10
YESHEX	RTS
NOTHEX	CLC
	RTS
*
BMTM	CMPB	#$5C
	BNE	BMTM2
	INX
	SEI
	LDAB	TIME2			    ;	TIME
	LDAA	TIME1
	CLI
	RTS
BMTM2	CMPB	#'^'
	BNE	BMTM4
	INX
	LDAB	0,X
	CMPB	#'$'
	BNE	BMTM3
	INX
	STX	LSWK
	JSR	CURPOS
	STX	LSWK2
	LDAA	LSWK2
	LDAB	LSWK2+1
	LDX	LSWK
	RTS
BMTM3	LDAA	CURX
	LDAB	CURY
	RTS
BMTM4	CMPB	#'['
	BNE	BMTM9
	INX
	JSR	KBIN
	BCC	BMTM41
	CLRA
BMTM41	TAB
	CLRA
	RTS
BMTM9	JMP	TERM3
*
*** "*READY" DATA ***
*
READY	FCB	$D,$D
;	FCB	$A
	FCC	'*READY'
	FCB	$D,0
STARTM	FCB	$C
	FCC	'GAME68/BM by zu &DATETIME'
	FCB	0
BREAKM	FCB	$7,$D
	FCC	'*BREAK'
	FCB	$0D,0
	
*****************************
*************************
*  LINE EDIT COMMAND    *
*  FOR 'GAME'           *
*************************
*
* (CHANGE 'GAME' PG)
*
* FROM
*  0161 CMPB #"/"
*  0163 BNE EDITER
* TO
*  0161 JMP LEDIT
*       NOP
*------------------------
*
* (COMMAND FORMAT)
*
* (LN)^OLD STR^NEW STR^
*
* EX. 200^A=B*C^B=B/D^
*     102^NL^^
*
*************************
*
NB2	EQU	W74
SP	EQU	W42
CP1	EQU	W64
CP	EQU	CP1+2
LP	EQU	CP+2
PP	EQU	LP+2
;NBUF	EQU	$0A00
*
;	ORG	$0A00
*
LEDIT	CMPB	#'/'
	BEQ	LIST1
	CMPB	#'^'
	BEQ	LINEDT
EDIT1	JMP	EDITER
LIST1	JMP	LIST
ERR	JMP	MAIN
EDT	LDX	#NBUF
	JSR	CRLF
	JSR	OUTLIN
	JSR	CRLF
	BRA	EDIT1
*
LINEDT	INX
	STX	CP1
	JSR	SRCLIN
	LDX	0,X
	STX	NBUF
	CPX	$4A
	BNE	ERR
	LDX	SP
	INX
	INX
	STX	SP
	LDX	#NBUF+2
	STX	NB2
	STX	LP
*
LE10	LDX	CP1
	STX	CP
	BSR	GETC1
LE20	BSR	TSPLP
	CBA
	BNE	LE20
	DEX
	STX	PP
LE30	BSR	GETC1
	CMPB	#'^'
	BEQ	LE40
	BSR	TSPLP
	CBA
	BEQ	LE30
	BRA	LE10
LE40	LDX	PP
	STX	LP
LE50	BSR	GETC1
	CMPB	#'^'
	BEQ	LE10
	TBA
	BSR	TLP
	BRA	LE50
*
TSPLP	LDX	SP
	LDAA	0,X
	INX
	STX	SP
TLP	LDX	LP
	STAA	0,X
	BEQ	EDT
	INX
	STX	LP
	RTS
*
GETC1	LDX	CP
	LDAB	0,X
	BEQ	ERR
	INX
	STX	CP
	RTS
*
LOAD	STX	LSWK
	LDX	PGB		    ; Program clear
	STX	PGE
	CLR	0,X
	DEC	0,X
	LDX	LSWK
APPEND	TPA
	STA A	FLOAD
LOAD1	INX
LOAD0	LDA	0,X
	CMPA	#' '
	BEQ	LOAD1
	TSTA
	BNE	LOAD11
LDERR	LDAA	#'['
	JMP	ERROR
LOAD11	LDA B	#6
	STA B	FLOAD		    ; LOAD Flag 0 or not 0
	CLR	LSWK2		    ; Filename -> $0043
	LDA A	#$43
	STA A	LSWK2+1
LOAD2	LDA A	0,X
	BEQ	LOAD3
	CMP A	#' '
	BEQ	LOAD3
	CMP A	#$0D
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
	JMP	PMODE
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
	CLR	FLOAD
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
*
SAVE	INX
	LDA	0,X
	CMPA	#' '
	BEQ	SAVE
	TSTA
	BNE	SAVE1
SVERR	LDAA	#'['
	JMP	ERROR
SAVE1	LDA B	#6
	CLR	LSWK2		    ; Filename -> $0043
	LDA A	#$43
	STA A	LSWK2+1
SAVE2	LDA A	0,X
	BEQ	SAVE3
	CMP A	#' '
	BEQ	SAVE3
	CMP A	#$0D
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
	STA A	FSAVE		    ; SAVE Flag 0 or not 0
	CLR	$004C
	LDX	#IOBUF
	STX	$003B
	STX	$003D
	LDX	LSWK
	LDX	PGB
	JMP	LLP1
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
INIT_MUSIC  CLR	$00C4
	CLR $00C5
	CLR $00C7
	CLR $00CA
	CLR $00CB
	DEC $00CB
	RTS
*
	IF	&BASICROM
	ORG	$C000
ROMSTRT	LDS	RAMEND
	JSR	NMISEI
	LDA A	#$7E
	LDX	#CHRGET
	STA A	ASCIN
	STX	ASCIN+1
	LDX	#CHROUT
	STA A	ASCOUT
	STX	ASCOUT+1
	LDX	#$F018
	STA A	BYTIN
	STX	BYTIN+1
	LDX	#$F01B
	STA A	BYTOUT
	STX	BYTOUT+1
	JMP	COLDST
	END	ROMSTRT
	ELSE
	END	COLDST
	ENDIF
;
; vim: set ts=8 noai:
;
