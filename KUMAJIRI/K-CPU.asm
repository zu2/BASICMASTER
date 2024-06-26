************************
*  K-CODE INTER PRETER *
*     VERSION 4.22     *
*                      *
* PROGRAMED BY N.TSUDA *
*      1980/5/11       *
*                      *
* TRANSPLANTATION TO   *
*   MB6881  BY JR6TEW. *
*      1980/10/8 WED.  *
************************
*
*
	ORG	$E0
PC	RMB	2
STACK	RMB	2
*
XSAVE	RMB	2
COUNT	RMB	1
SFLAG	RMB	1
WARK	RMB	2
WORK2	RMB	2
ASCII	RMB	6
RAND	RMB	2
*
OUT	EQU	$F015
IN	EQU	CHRGET
*
IS	EQU	$17FE
BUFFER	EQU	$400
*
	ORG	$B00
*****************************
* M A I N R O U T I N E
*	E - T Y P E
*****************************
	PULA
	PULB
	STAA	PC
	STAB	PC+1
	LDX	#IS
	STX	STACK
	LDX	PC
EX	STAA	WARK
	LDAA	,X
	INX
	STX	PC
	ASLA
	LDX	#TABLE
	STAA	*+6
	LDAA	WARK
	LDX	,X
	JSR	,X
	BRA	EX
*
RET68	LDX	PC
	INS
	INS
	JMP	,X
*
CRE	LDX	PC
CR	LDAA	#$D
	JMP	OUT
*
PRM	LDX	PC
	BRA	PRM2
PLOOP	JSR	OUT
PRM2	LDAA	,X
	INX
	BPL	PLOOP
	ANDA	#$7F
	JMP	OUT
*
IFOR	LDX	STACK
	TBA
	LDAB	3,X
	STAA	3,X
	PSHB
	LDAA	WARK
	TAB
	LDAA	2,X
	STAB	2,X
	PULB
	LDX	PC
	INX
	STX	PC
	LDX	,X
	BRA	FOR2
*
FOR	LDX	PC
	LDX	,X
	LDAA	,X
	LDAB	1,X
	ADDB	#1
	ADCA	#0
FOR2	STAA	,X
	STAB	1,X
	LDX	STACK
	SUBB	3,X
	SBCA	2,X
	BGT	FORO
	BNE	FORC
	TSTB
	BEQ	FORC
FORO	LDX	STACK
	INX
	INX
	STX	STACK
	LDX	PC
	LDX	2,X
	RTS
FORC	LDX	PC
	INX
	INX
	BRA	IN2
*
GOSUB	LDAA	PC
	LDAB	PC+1
	ADDB	#2
	ADCA	#0
	LDX	STACK
	STAA	,X
	STAB	1,X
	DEX
	DEX
	STX	STACK
*
GOTO	LDX	PC
	LDX	,X
	RTS
*
RET	LDX	STACK
	INX
	INX
	STX	STACK
	LDX	,X
	RTS
*
IF	LDX	PC
	TSTB
	BNE	IN2
	TSTA
	BNE	IN2
	LDX	,X
	RTS
*
PLDC	BSR	PUSH
LDC	LDX	PC
	LDAA	,X
	LDAB	1,X
IN2	INX
	INX
	RTS
*
LARY1	BSR	ARRAY1
	CLRA
	LDAB	,X
	LDX	PC
	BRA	IN2
*
LARY2	BSR	ARRAY2
	BRA	LD02
*
PLDV	BSR	PUSH
LDV	LDX	PC
	LDX	,X
LD02	LDAA	,X
	LDAB	1,X
IN2P	LDX	PC
	INX
	INX
	RTS
*
PUSH	LDX	STACK
	DEX
	DEX
	STX	STACK
	STAA	2,X
	STAB	3,X
	RTS
*
ARRAY2	ASLB
	ROLA
ARRAY1	LDX	PC
	LDX	,X
	ADDB	1,X
	ADCA	,X
	STAA	XSAVE
	STAB	XSAVE+1
	LDX	XSAVE
	RTS
*
SARY1	PSHB
	BSR	PULL
	BSR	ARRAY1
	PULB
	STAB	,X
	BRA	IN2P
*
SARY2	PSHA
	PSHB
	BSR	PULL
	BSR	ARRAY2
	PULB
	PULA
	BRA	ST02
*
STORE	LDX	PC
	LDX	,X
ST02	STAA	,X
	STAB	1,X
	LDX	PC
	INX
	INX
	RTS
*
PULL	LDX	STACK
	INX
	INX
	STX	STACK
	LDAB	1,X
	LDAA	,X
	RTS
*
ADD	LDX	STACK
	INX
	INX
	STX	STACK
	ADDB	1,X
	ADCA	,X
LDP	LDX	PC
	RTS
*
SUB	BSR	ROPS
*
NEG	LDX	PC
NEGS	COMA
	NEGB
	BNE	NRET
	INCA
NRET	RTS
*
ABS	TSTA
	BMI	NEG
	BRA	LDP
*
ROPS	LDX	STACK
	INX
	INX
	STX	STACK
	SUBB	1,X
	SBCA	,X
	RTS
*
BGE	BSR	ROPS
	BGT	GO
	BNE	IN2PP
	TSTB
	BNE	GO
	BRA	IN2PP
*
BLT	BSR	ROPS
	BLT	GO
	BNE	IN2PP
	TSTB
	BEQ	GO
	BRA	IN2PP
*
BGT	BSR	ROPS
	BGE	GO
	BRA	IN2PP
*
BLE	BSR	ROPS
	BLT	GO
	BRA	IN2PP
*
BEQ	BSR	ROPS
	BNE	GO
	TSTB
	BNE	GO
	BRA	IN2PP
*
BNE	BSR	ROPS
	BNE	IN2PP
	TSTB
	BNE	IN2PP
*
GO	LDX	PC
	LDX	,X
	RTS
*
IN2PP	LDX	PC
	INX
	INX
	RTS
*
GE	BSR	ROPS
	BGT	F
	BNE	T
	TSTB
	BNE	F
	BRA	T
*
LT	BSR	ROPS
	BLT	F
	BNE	T
	TSTB
	BEQ	F
	BRA	T
*
GT	BSR	ROPS
	BLT	T
	BRA	F
*
LE	BSR	ROPS
	BLT	F
	BRA	T
*
NOT	TSTA
	BRA	*+4
*
EQ	BSR	ROPS
	BNE	F
	TSTB
	BNE	F
	BRA	T
*
NE	BSR	ROPS
	BNE	T
	TSTB
	BEQ	F
*
T	LDAB	#1
	FCB	$81
F	CLRB
	CLRA
	LDX	PC
	RTS
*
MUL	LDX	STACK
	INX
	INX
	STX	STACK
	BSR	MULT
	LDX	PC
	RTS
*
MULT	STAA	WARK
	STAB	WARK+1
	LDAA	#16
	STAA	COUNT
	CLRA
	CLRB
	STAA	SFLAG
	BRA	M01
MLOOP	ASLB
	ROLA
	ROL	SFLAG
M01	ASL	WARK+1
	ROL	WARK
	BCC	M02
	ADDB	1,X
	ADCA	,X
	BCC	M02
	INC	SFLAG
M02	DEC	COUNT
	BNE	MLOOP
	RTS
*
DIV	LDX	#0
	STX	COUNT
	STX	WORK2
	TSTA
	BPL	D02
	JSR	NEGS
	COM	SFLAG
D02	INC	COUNT
	BMI	ZDERR
	ASLB
	ROLA
	BPL	D02
	LSRA
	RORB
	STAA	WARK
	STAB	WARK+1
	JSR	PULL
	BPL	D03
	JSR	NEGS
	COM	SFLAG
D03	DEC	COUNT
	BMI	D04
	ASLB
	ROLA
 	BPL	D03
	LSRA
	RORB
D06	SUBB	WARK+1
	SBCA	WARK
	BRA	D08
DLOOP	ASLB
	ROLA
	BCC	D06
	ADDB	WARK+1
	ADCA	WARK
D08	CLC
	BMI	D09
	SEC
D09	ROL	WORK2+1
	ROL	WORK2
	DEC	COUNT
	BPL	DLOOP
D04	LDAA	WORK2
	LDAB	WORK2+1
	TST	SFLAG
	BPL	DRET
	JMP	NEG
*
ZDERR	LDAA	#$7F
	LDAB	#$FF
DRET	LDX	PC
	RTS
*
PGET	JSR	PUSH
GET	JSR	IN
	JSR	OUT
	TAB
	CLRA
	LDX	PC
	RTS
*
PINPUT	JSR	PUSH
INPUT	LDX	#BUFFER
I01	STX	XSAVE
	JSR	IN
	LDX	XSAVE
	STAA	,X
	CMPA	#$7F
	BEQ	CLR
	CMPA	#$D
	BEQ	I02
	JSR	OUT
	INX
	BRA	I01
CLR	CPX	#BUFFER
	BEQ	I01
	JSR	OUT
	DEX
	BRA	I01
I02	CPX	#BUFFER
	BEQ	I01
	JSR	OUT
	LDX	#BUFFER
	STX	WARK
 	LDX	#WORK2
	CLR	,X
	CLR	1,X
	BSR	INCHR
	CMPA	#$24	"$"
	BEQ	HEX
	CMPA	#$2D	"-"
	BNE	DCHECK
	BSR	DC02
	JMP	NEGS
DC01	SUBA	#$30	"0"
	STAA	COUNT
	LDAA	,X
	LDAB	1,X
	ASLB
	ROLA
	ASLB
	ROLA
	ADDB	1,X
	ADCA	,X
	ASLB
	ROLA
	ADDB	COUNT
	ADCA	#0
	STAA	,X
	STAB	1,X
DC02	BSR	INCHR
DCHECK	CMPA	#$30
	BCS	I03
	CMPA	#$39
	BHI	I03
	BRA	DC01
I03	LDAA	,X
	LDAB	1,X
	LDX	PC
	RTS
*
H02	ASL	1,X
	ROL	,X
	ASL	1,X
	ROL	,X
	ASL	1,X
	ROL	,X
	ASL	1,X
	ROL	,X
	ADDA	1,X
	STAA	1,X
HEX	BSR	INCHR
	SUBA	#$30
	BCS	I03
	CMPA	#$0A
	BCS	H02
	CMPA	#$11
	BCS	I03
	CMPA	#$17
	BCC	I03
	SUBA	#$07
	BRA	H02
*
INCHR	STX	XSAVE
	LDX	WARK
	LDAA	,X
	INX
	STX	WARK
	LDX	XSAVE
	RTS
*
PRINTR	BSR	TRBA

	STX	WARK
	LDX	STACK
	INX
	INX
	STX	STACK
	LDAB	1,X
	ADDB	WARK+1
	SUBB	#6
	BLS	P04
	BSR	SPOUT
P04	LDX	WARK
 	FCB	$8C
PRINT	BSR	TRBA
PL	LDAA	ASCII,X
	JSR	OUT
	INX
	CPX	#6
	BNE	PL
	LDX	PC
	RTS
*
TAB	LDX	PC
SPOUT	LDAA	#$20	" "
SP02	JSR	OUT
	DECB
	BNE	SP02
	RTS
*
* TRANSFORM BINARY TO ASCII
*
TRBA	TSTA
	BPL	BINASC
	JSR	NEGS
	BSR	BINASC
	LDAA	#$2D	"-"
	DEX
	STAA	ASCII,X
	RTS
*
BINASC	ASLB
	ROLA
	STAA	WARK
	STAB	WARK+1
	LDX	#$3030
	STX	ASCII
	STX	ASCII+2
	STX	ASCII+4
	LDAB	#15
	BRA	B05
BL	LDX	#5
	CLC
B02	LDAA	ASCII,X
	ROLA
	CMPA	#$69
	BHI	B03
	SUBA	#$30	"0"
	BRA	B04
B03	SUBA	#$3A
	SEC
B04	STAA	ASCII,X
	DEX
	BNE	B02
B05	ASL	WARK+1
	ROL	WARK
	BCC	*+5
	INC	ASCII+5
	DECB
	BNE	BL
*
* ZERO BLANKING
*
	LDAA	#$30	"0"
	LDX	#1
ZL	CMPA	ASCII,X
	BNE	ZRET
	INX
	CPX	#5
	BNE	ZL
ZRET	RTS
*
PRFHEX	PSHB
	BSR	HEXOUT
	PULB
PRTHEX	TBA
	LDX	PC
HEXOUT	JSR	OUTHEX
	RTS
*
PRCHR	TBA
	LDX	PC
	JMP	OUT
*
AND	BSR	INS
	ANDB	1,X
	ANDA	,X
	BRA	LRET
*
EOR	BSR	INS
	EORB	1,X
	EORA	,X
	BRA	LRET
*
OR	BSR	INS
	ORAB	1,X
	ORAA	,X
LRET	LDX	PC
	RTS
*
INS	LDX	STACK
	INX
	INX
	STX	STACK
	RTS
*
RND	LDX	#RAND
	PSHA
	PSHB
	LDAA	#$3D
	LDAB	#9
	JSR	MULT
	ADDB	#3
	ADCA	#0
	STAA	RAND
	STAB	RAND+1
	PULB
	PULA
	JSR	MULT
	CLRA
	LDAB	SFLAG
	BRA	LRET
*
SYS	STAA	XSAVE
	STAB	XSAVE+1
	LDX	XSAVE
	JSR	,X
	BRA	LRET
*
STOP	INS
	INS
	RTS
*
BADDC	LDX	PC
	ADDB	1,X
	ADCA	,X
	INX
	INX
	RTS
*
BADDV	LDX	PC
	LDX	,X
	ADDB	1,X
	ADCA	,X
	JMP	IN2P
*
BSUBC	LDX	PC
	SUBB	1,X
	SBCA	,X
	INX
	INX
	RTS
*
BSUBV	LDX	PC
	LDX	,X
	SUBB	1,X
	SBCA	,X
	JMP	IN2P
*
* INSTRACTION TABLE
*
TABLE	FDB	GOTO
	FDB	GOSUB
	FDB	RET
	FDB	IF
	FDB	IFOR
	FDB	FOR
	FDB	STOP
*
	FDB	LDC
	FDB	PLDC
	FDB	LDV
	FDB	PLDV
	FDB	LARY1
	FDB	LARY2
	FDB	STORE
	FDB	SARY1
	FDB	SARY2
*
	FDB	NOT
	FDB	NEG
	FDB	ABS
	FDB	RND
*
	FDB	MUL
	FDB	DIV
	FDB	ADD
	FDB	SUB
	FDB	GT
	FDB	GE
	FDB	LT
	FDB	LE
	FDB	EQ
	FDB	NE
*
	FDB	AND
	FDB	OR
	FDB	EOR
*
	FDB	BGT
	FDB	BGE
	FDB	BLT
	FDB	BLE
	FDB	BEQ
	FDB	BNE
*
	FDB	PRM
	FDB	CRE
	FDB	PRINT
	FDB	PRINTR
	FDB	PRFHEX
	FDB	PRTHEX
	FDB	PRCHR
	FDB	TAB
*
	FDB	PGET
	FDB	GET
	FDB	PINPUT
	FDB	INPUT
*
	FDB	SYS
	FDB	BADDC
	FDB	BADDV
	FDB	BSUBC
	FDB	BSUBV
	FDB	RET68
*
*****************************
*
* I/O RUTIN OF MB6881
*
*  BY JR6TEW
*
* INPUT
CHRGET	LDAA	#$5F
	JMP	$F012
* HEXOUT
OUTHEX	PSHB
	PSHA
	PSHA
	BSR	OH1
	PULA
	BSR	OH2
	PULA
	PULB
	RTS
OH1	LSRA
	LSRA
	LSRA
	LSRA
OH2	ANDA	#$F
	ADDA	#$30
	CMPA	#$39
	BLS	OH3
	ADDA	#7
OH3	JMP	OUT
*
*
	END
*
*
;
; vim: set ts=8 noai:
;
