********************************
*
*   RANPAKU TEXT EDITOR
*     VERSION 3.2
*
*   PROGRAMMED BY N.TSUDA
*       1979/11/29
*
*   Port to BASICMASTER
*	by ZUKERAN, shin (@zu2)
*	2024/06/XX
*
********************************
BUFFER	EQU	$1600
	ORG	$00E0
POINTR	RMB	2
XSAVE	RMB	2
SSAVE	RMB	2
WARK	RMB	1
FLAG	RMB	1
XWORK	RMB	2
*
RAMEND	EQU	$0008
ASCIN	EQU	$0028
ASCOUT	EQU	$002B
BYTIN	EQU	$002E
BYTOUT	EQU	$0031
ADRTOP	EQU	$003B
ADREND	EQU	$003D
FNAME	EQU	$0043
;INPUTP	EQU	$190A
MONITOR	EQU	$F000
KBIN	EQU	$F00F
MON_GO	EQU	$F476
GETFNAM	EQU	$F682
NMISET	EQU	$FFE6
NMICLR	EQU	$FFE9
CLRTV	EQU	$FFEF
*
KCOMP	EQU	$2000
*
	ORG	$1800
	BRA	CSTART
*
* HOT START
*
	JSR	CR
	BRA	HSTART
*
MEMBEG	FDB	$3400-1
MEMEND	FDB	$4FFF
TXTEND	RMB	2
*
CSTART	JSR	CLRTV
	LDX	#IMES
	JSR	OUTSTR
	JSR	CR
	JSR	CR
	LDX	MEMBEG
	STX	TXTEND
	BRA	BEGIN
*
HSTART	LDX	MEMBEG
	LDAA	#$D
	LDAB	#$FF
HS1	INX
	CMPA	0,X
	BNE	HS1
	CMPB	1,X
	BNE	HS1
	STX	TXTEND
BEGIN	LDX	MEMBEG
	STX	POINTR
EDITOR	LDS	RAMEND
	JSR	NMISET
	LDX	TXTEND
	CPX	MEMBEG
	BEQ	ED03
	LDX	POINTR
	FCB	$8C
	LDX	XWORK		; Where come from???
ED02	STX	POINTR
	CPX	TXTEND
	BEQ	ED03
	JSR	OUTS1		; out string from (IX). set IX to EOL
	JSR	INPUT		; AccA=getch()
	PSHA
	JSR	CR
	PULA
	CMPA	#$0A		; ↓: show next line
	BEQ	ED02
	CMPA	#$09		; →: show next line
	BEQ	ED02
	CMPA	#$0D		; CR: show next line
	BEQ	ED02
	CMPA	#$0B		; ↓: show prev line
	BEQ 	ED020
	CMPA	#$08		; ←: show prev line
	BNE	ED021
ED020	JSR	U01
	BRA	ED02
ED021	CMPA	#$7F		; don't show DEL
	BEQ	ED02
ED022	CMPA	#$10
	BCS	ED02		; don't shot contorl code
	LDX	#0
	STX	XWORK		; clear Number of lines
	TAB
	LDAA	#':'
	JSR	OUT
	TBA
	BSR	OUT		; echo input char
	BRA	ED04
ED03	LDAA	#':'
	BSR	OUT
	LDX	#0
	STX	XWORK		; clear Number of lines
INCOM	BSR	INPUT		; getch(), no echo
	CMPA	#$0B		; ↑
	BEQ	INCOMU
	CMPA	#$08		; ←
	BNE	INCOM1
INCOMU	LDAA	#'U'
	BRA	INCOM9
INCOM1	CMPA	#$0A		; ↓
	BEQ	INCOMD
	CMPA	#$09		; →
	BNE	INCOM2
INCOMD	LDAA	#'D'
	BRA	INCOM9
INCOM2	CMPA	#$10		; don't echo contorl code
	BCS	INCOM
	CMPA	#$7F		; don't echo DEL
	BEQ	INCOM
INCOM9	BSR	OUT		; echo input char
ED04	CMPA	#$30		; 0-9?
	BCS	ED05
	CMPA	#$3A
	BCC	ED05
ED041	LDAB	XWORK+1		; (XWORK,WORK+1) ←[0-9][0-9] max 2digit
	STAB	XWORK
	ANDA	#$F
	STAA	XWORK+1
	BRA	INCOM
ED05	PSHA			; execute command (not 0-9)
	BSR	CR
	LDAA	XWORK+1		; calculate nn
	LDAB	XWORK
	JSR	MUL10		; AccA=AccB*10+AccA
	TAB
	BNE	ED051
	INCB			; if nn is omitted, nn=1
ED051	PULA			; AccA:Command, AccB:Number of lines(nn)
	LDX	#CTABLE
ED06	CMPA	0,X
	BEQ	JMPCOM
	INX
	INX
	INX
	CPX	#TBLEND
	BNE	ED06
	LDX	#HELPMES
	JSR	OUTSCR
BEDIT	JMP	EDITOR
JMPCOM	LDX	1,X
	JSR	0,X
	BRA	BEDIT
INPUT	LDAA	#'_'
	JMP	ASCIN
;INPUT	STX	INXS+1
;	LDAA	#5
;	STAA	$E007	;PIA
;	CLI
;	JSR	INPUTP
;	SEI
;INXS	LDX	#0
LIRET	RTS
*
IN	BSR	INPUT
	FCB	$8C
CR	LDAA	#$D
OUT	JMP	ASCOUT
*
* INPUT ONE LINE to BUFFER
*
CCR	BSR	CR	; CR and clear buffer
LININ	LDAA	#'>'
	BSR	OUT
	LDX	#BUFFER
	CLRB
L1B	BSR	INPUT	; getch without echo
	CMPA	#$5F	; '_'		; CLR LINE
	BEQ	CCR
	CMPA	#$7F	; DEL		; #$7E [CLR]
	BNE	L1D
L1C	BSR	OUT	; echo
	DECB
	BMI	LININ
	DEX
	BRA	L1B
L1D	CMPA	#$0D
	BEQ	L1D1
	CMPA	#$10	; ignore control char.
	BCS	L1B
L1D1	BSR	OUT
	STAA	,X
	INX
	INCB
	CMPA	#$0D	; CR: End Edit
	BEQ	L1E
	CMPB	#100	; LINE too long.
	BNE	L1B
	BRA	CCR
L1E	CMPB	#1	; empty line
	BEQ	BEDIT
	LDAA	#$FF	; insert a line to text
	SUBA	TXTEND+1
	STAA	XWORK
	LDAA	MEMEND
	SBCA	TXTEND
	BHI	LIRET
	CMPB	XWORK
	BLS	LIRET
	LDX	#FULL
	JSR	OUTSTR
	BSR	CR
	BRA	BEDIT
*
* CHANGE LINE
*
CCOM	LDX	POINTR
	CPX	TXTEND
	BNE	C01
CERR	LDX	#CEMES
	JSR	OUTSTR
	BRA	CR
C01	BSR	LININ
CP	PSHB
	LDX	POINTR
	CLRB
	LDAA	#$D
L20	INCB
	INX
	CMPA	0,X
	BNE	L20
	LDAA	TXTEND+1
	SBA
	STAA	TXTEND+1
	BCC	L201
	DEC	TXTEND
L201	LDX	POINTR
	INCB
	STAB	L22+1
	BRA	L62
L22	LDAA	0,X
	STAA	1,X
	INX
L62	CPX	TXTEND
	BNE	L22
	PULB
	BRA	ISUB
*
* APPEND
*
ACOM	JSR	LININ
	BSR	ISUB
	STX	POINTR
	BRA	ACOM
*
* INSERT
*
ICOM	JSR	LININ
ISUB	STAB	L25+1
	LDX	TXTEND
	BRA	L5A
L24	LDAA	0,X
L25	STAA	0,X
	DEX
L5A	CPX	POINTR
	BNE	L24
	ADDB	TXTEND+1
	STAB	TXTEND+1
	BCC	L5A1
	INC	TXTEND
L5A1	LDX	#BUFFER
L27	LDAA	0,X
	INX
	STX	XWORK
	LDX	POINTR
	STAA	1,X
	INX
	STX	POINTR
	LDX	XWORK
	CMPA	#$D
	BNE	L27
	LDX	POINTR
	RTS
*
* LIST
*
LCOM	LDAB	#24
*
* DOWN POINTR
*
DCOM	LDX	POINTR
D01	CPX	TXTEND
	BEQ	DRET
	PSHA
	JSR	OUTS1CR
	PULB
	STX	POINTR
	DECB
	BNE	D01
DRET	RTS
*
* FIND
*
FCOM	JSR	LININ
	DECB
	STAB	WARK
	LDX	POINTR
F02	CPX	TXTEND
	BEQ	DRET
	STS	SSAVE
	SEI
	LDS	POINTR
F033	LDAB	WARK
	LDX	#BUFFER-1
F03	PULA
	CMPA	#' '
	BEQ	F03
	INX
	CMPA	0,X
	BNE	F04
	DECB
	BEQ	F05
	CMPA	#$D
	BNE	F03
F04	PULA
	CMPA	#' '
	BEQ	F033
	CMPA	#$D
	BNE	F04
	LDX	POINTR
	LDS	SSAVE
	CLI
	JSR	OUTS1CR
	STX	POINTR
	BRA	F02
F05	LDS	SSAVE
	CLI
	RTS
*
* KILL
*
KCOM	PSHB		    ; save Number of Lines
	LDX	#OK
	JSR	OUTSTR
	JSR	INPUT
	TAB
	JSR	CR
	PULA
	CMPB	#'Y'
	BEQ	K01
	CMPB	#$0D
	BNE	KRET
K01	LDAB	#$0D
	LDX	POINTR
K02	CPX	TXTEND
	BEQ	K03
	INX
	CMPB	0,X
	BNE	K02
	DECA
	BNE	K02
	STX	XSAVE
	STS	SSAVE
	SEI
	TXS
	INS
	LDAB	TXTEND+1
	LDAA	TXTEND
	SUBB	XSAVE+1
	SBCA	XSAVE
	ADDB	POINTR+1
	ADCA	POINTR
	STAA	TXTEND
	STAB	TXTEND+1
	LDX	POINTR
L3B	CPX	TXTEND
	BEQ	L3C
	PULA
	INX
	STAA	0,X
	BRA	L3B
L3C	LDS	SSAVE
	CLI
KRET	RTS
*
* END OF BUFFER
*
ZCOM	EQU	*
K03	LDX	TXTEND
	STX	POINTR
	RTS
OK	FCC	" OK ?"
	FCB	$D
*
* UP POINTR
*
U01	LDAB	#1
UCOM	LDX	POINTR
	LDAA	#$D
U02	CPX	MEMBEG
	BEQ	URET
	DEX
	CMPA	0,X
	BNE	U02
	DECB
	BNE	U02
URET	STX	POINTR
	RTS
*
* LINE EDIT
*
ECOM	LDX	POINTR	    ; Current LINE Top
	CPX	TXTEND
	BNE	E02
	JMP	CERR
E02	INX
	STX	XWORK
	LDX	#BUFFER-1
	CLRB
	STAB	FLAG	    ; 変更したか?
EDLINE	JSR	INPUT	    ; AccA=getch()
	CMPA	#$0D	    ; CR
	BNE	ED1
* CR: END EDIT
	TST	FLAG
	BNE	ED1N
* Transfer the rest of the current line to the BUFFER
EDC	JSR	INCHR	    ; AccA=(*XWORK++)
	INCB
	INX
	STAA	0,X
	CMPA	#$0D	    ; CR
	BEQ	JC
	JSR	OUT
	BRA	EDC
JC	JSR	CR
	STAA	0,X
	JMP	CP	    ; CHANGE Line
ED1	CMPA	#$09	    ; #$7D  →
	BNE	ED2
ED11	JSR	INCHR	    ; AccA=(*XWORK++)
ED1N	INCB
ED1P	INX
	STAA	0,X	    ; store AccA into line buffer
	CMPA	#$D
	BEQ	JC
	JSR	OUT
	BRA	EDLINE
ED2	CMPA	#$7F	    ; 後退	; #$5F	delete a char
	BNE	ED3
	JSR	INCHR	    ; AccA=(*WORK++)
	CMPA	#$0D	    ; CR
	BNE	EDLINE
	CLRA
	BRA	ED1N
ED3	CMPA	#$5F	    ; '_'	; #$7F	all delete
	BNE	ED4
* CUT LINE
	INCB
	INX
	BRA	JC
ED4	CMPA	#$0B	    ; ↑	; #$5E  '^' ignore edit.
	BNE	ED5
* CANCEL
	JMP	CR
ED5	CMPA	#$08	    ; ←	; #$7E	move cursor left
	BNE	ED6
* BACK SPACE
	DECB
	BMI	ECOM
	DEX
	LDAA	XWORK
	BNE	ED51
	DEC	XWORK
ED51	DEC	XWORK+1
	LDAA	#$7F	    ; 後退	; #$7E 
	JSR	OUT
EDLINP	BRA	EDLINE
ED6	CMPA	#$0A	    ; #$7C ↓
	BNE	ED7
* INSERT a char
	JSR	INPUT
	CMPA	#$10
	BCS	EDLINP
	CMPA	#$7F	    ; #$7E
	BEQ	EDLINP
	BRA	ED1N
* NOT control code. just replace it.
ED7	TST	FLAG
	BNE	ED1N
	PSHA
	JSR	INCHR	    ; AccA=(*WORK++)
	CMPA	#$0D	    ; EOL
	BNE	ED8
	INC	FLAG
ED8	PULA
	BRA	ED1N
*
EOFF	LDAA	#$FF
	FCB	$81
EOF0	CLRA
	LDX	TXTEND
	STAA	1,X
	RTS
* eXtra command
XCOM	BSR	EOFF
	LDX	#EXCOMM
	JSR	OUTSTR
	JSR	INPUT
	CMPA	#'M'
	BNE	XCGO
XCMON	JMP	MONITOR
XCGO	CMPA	#'G'
	BNE	XCCOMP
	JMP	MON_GO
XCCOMP	CMPA	#'C'
	BNE	XCSAVE
	JMP	KCOMP
XCSAVE	CMPA	#'S'
	BNE	XCLOAD
	BSR	EOF0
	JSR	GETFNAM		; get Filename
	LDX	MEMBEG
	INX
	STX	ADRTOP
	LDX	TXTEND
	INX
	STX	ADREND
	LDAA	#'S'
	STAA	FNAME+7
	JSR	BYTOUT
	JSR	CR
	BRA	XCOME
XCLOAD	CMPA	#'L'
	BNE	XCOME
	JSR	$F682		; get Filename
	LDX	MEMBEG
	INX
	STX	ADRTOP
	LDAA	#'S'
	STAA	FNAME+7
	JSR	BYTIN
	LDX	ADRTOP
	DEX
	STX	MEMBEG
	LDX	ADREND
	FCB	$81
XCLOAD2	DEX
	LDAA	0,X
	CMPA	#$0D
	BNE	XCLOAD2
XCLOAD3	STX	TXTEND
	BSR	EOFF
	JSR	CR
	JMP	HSTART
XCOME	JMP	EDITOR 
*
PAUSE	STX	PX+1
	LDX	#$5000
PA01	DEX
	BNE	PA01
PX	LDX	#0
	RTS
*
IMES	FCC	" - TEXT EDITOR VER 3.2 -"
	FCB	$D
FULL	FCC	" BUFFER IS FULL"
	FCC	" OF PROGRAM."
	FCB	$D
CEMES	FCC	" THERE ISN'T LINE."
	FCB	$D
HELPMES	FCC	" Append/Begin/Change/Down/Edit/Find/Insert/Kill/List/Tail/Up/eXtra."
	FCB	$D
EXCOMM	FCC	" Monitor/Save/Load/Compile/Go?"
	FCB	$D
CTABLE	FCB	"A"
	FDB	ACOM
	FCB	"B"
	FDB	BEGIN
	FCB	"C"
	FDB	CCOM
	FCB	"D"
	FDB	DCOM
	FCB	"F"
	FDB	FCOM
	FCB	"L"
	FDB	LCOM
	FCB	"I"
	FDB	ICOM
	FCB	"T"
	FDB	ZCOM
	FCB	"Z"
	FDB	ZCOM
	FCB	"K"
	FDB	KCOM
	FCB	"U"
	FDB	UCOM
	FCB	"E"
	FDB	ECOM
	FCB	"X"
	FDB	XCOM
	FCB	"["
	FDB	XCOM
TBLEND	*
OUTSCR	BSR	OUTSTR
	FCB	$8C
OUTS1CR	BSR	OUTS1
OUTS9	JMP	ASCOUT
OUTS0	BSR	OUTS9
OUTS1	INX
OUTSTR	LDAA	0,X
	CMPA	#$0D
	BNE 	OUTS0
	RTS
INCHR	STX	XSAVE
	LDX	XWORK
	LDAA	0,X
	INX
	STX	XWORK
	LDX	XSAVE
	RTS
*
*   AccA = AccB*10
*
MUL10	PSHA
	TBA
	ASLA
	ASLA
	ABA
	ASLA
	PULB
	ABA
	RTS
	END
;
; vim: set ts=8 noai:
;
