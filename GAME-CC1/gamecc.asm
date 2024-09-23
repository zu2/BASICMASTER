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
;
;	MOD(%)演算をシフトに置き換える
;	AB = AB%const
;
;	cf. https://camidion.wordpress.com/2012/11/10/speedy_modulo/
		IF		IF_MOD7
MOD7	TSTA				; 2 1
		BPL		MOD7L0		; 4 2
		NEGA				; 2 1
		NEGB				; 2 1
		SBCA	#0			; 2 2
MOD7L0	BNE		MOD7L1		; 4 2
		BITB	#$F8		; 2 2
		BEQ		MOD7L3		; 4 2
MOD7L1	STAB	W66+1		; 4 2	save Breg
		ANDB	#7			; 2 2
		STAB	W66			; 4 2	W66=num&7
		LDAB	W66+1		; 3 2
		LSRA				; 2 1
		RORB				; 2 1
		LSRA				; 2 1
		RORB				; 2 1
		LSRA				; 2 1
		RORB				; 2 1	AB>>3
		ADDB	W66			; 4 2
		ADCA	#0			; 2 2
MOD7L2	BNE		MOD7L1		; 4 2
		BITB	#$F8		; 2 2
		BNE		MOD7L1		; 4 2
MOD7L3	CMPB	#7			; 2 2
		BCS		MOD7E		; 4 2
		SUBB	#7			; 2 2
MOD7E	CLRA
		RTS					; 5 1
		ENDIF
;
;
;
		IF		IF_MOD15
MOD15	TSTA				; 2 1
		BPL		MOD15L0		; 4 2
		NEGA				; 2 1
		NEGB				; 2 1
		SBCA	#0			; 2 2
MOD15L0	BNE		MOD15L1		; 4 2
		BITB	#$F0		; 2 2
		BEQ		MOD15L3		; 4 2
MOD15L1	STAB	W66+1		; 4 2	save Breg
		ANDB	#15			; 2 2
		STAB	W66			; 4 2	W66=num&15
		LDAB	W66+1		; 3 2
		LSRA				; 2 1
		RORB				; 2 1
		LSRA				; 2 1
		RORB				; 2 1
		LSRA				; 2 1
		RORB				; 2 1
		LSRA				; 2 1
		RORB				; 2 1	AB>>4
		ADDB	W66			; 4 2
		ADCA	#0			; 2 2
MOD15L2	BNE		MOD15L1		; 4 2
		BITB	#$F0		; 2 2
		BNE		MOD15L1		; 4 2
MOD15L3	CMPB	#15			; 2 2
		BCS		MOD15E		; 4 2
		SUBB	#15			; 2 2
MOD15E	CLRA
		RTS					; 5 1
		ENDIF
;
;	cf. https://stackoverflow.com/questions/76197520/is-it-possible-to-make-x10-modulus-10-with-bitwise-operators-in-c
;
;    n = (n>>8) + (n & 0xff);
;    n = (n>>4) + (n & 0x0f);
;    n = (n>>4) + (n & 0x0f);
;    if (n >= 10) n -= 10;
;    if (n >= 5) n -= 5;
;
		IF		IF_MOD5+IF_MOD10
MOD5	TSTA				; 2 1
		BPL		MOD5L0		; 4 2
		NEGA				; 2 1
		NEGB				; 2 1
		SBCA	#0			; 2 2
MOD5L0	STAA	W66+1		; 4 2 W66=high(n)
		CLRA				; 2 1
		ADDB	W66+1		; 3 2 
		ADCA	#0			; 2 2 n=(n>>8)+(n&0xff)
		STAB	W66			; 3 2 save low byte
		LSRA				; 2 1 AccA is at most 1, so you only need one shift.
		RORB				; 2 1
		LSRB				; 2 1
		LSRB				; 2 1
		LSRB				; 2 1
		LDAA	W66			; 3 2 Since n<=255 ,so AccA is not used, calculate n&0x0f
		ANDA	#$0F		; 2 2
		ABA					; 2 1 n = (n>>4) + (n & 0x0f);
		TAB					; 2 1
		ANDA	#$0F		; 2 2
		LSRB				; 2 1
		LSRB				; 2 1
		LSRB				; 2 1
		LSRB				; 2 1
		ABA					; 2 1 n = (n>>4) + (n & 0x0f);
MOD5L1	TAB					; 2 1
		SUBB	#10			; 2 2
		BCC		MOD5L2		; 4 2
		TAB					; 2 1
MOD5L2	TBA					; 2 1
		SUBB	#5			; 2 2
		BCC		MOD5L3		; 4 2
		TAB					; 2 1
MOD5L3	CLRA				; 2 1
		RTS					; 5 1
		ENDIF
;
;    return (mod5(n>>1)<<1) | (n&1);
;
		IF		IF_MOD10
MOD10	TSTA				; 2 1
		BPL		MOD10L0		; 4 2
		NEGA				; 2 1
		NEGB				; 2 1
		SBCA	#0			; 2 2
MOD10L0	STAB	W68			; 4 2	
		LSRA				; 2 1
		RORB				; 2 1
		BSR		MOD5L0		; 8 2
		ROR		W68			; 6 3
		ROLB				; 2 1
MOD10E	RTS					; 5 1
		ENDIF
;
;	TODO: other mod xx pattern.
;		cf. https://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml	MOD 3,6,7
;
*
*	MULTIPLY Dr Jefyll's method
*			cf. http://forum.6502.org/viewtopic.php?p=19958#p19958
*	MULTIPLY	AB = (IX,IX+1)*AB	;	The destination indicated by IX will not be destroyed.
*	MULTIPLYX	AB = IX*AB			;	The destination indicated by IX will not be destroyed.
*
MULTIPLY	LDX		0,X				;   6 2		呼び出し元でW66に入れてから呼びたいが難しい
MULTIPLYX	STX		W66				;	4 2
			STAB	W68+1			;	4 2
			CLRB					;   2 1
			STAA	W68				;	4 2
			BEQ		MULTI02			;   4 2
			CLRA					;   2 1
			LDX		#8              ;   3 3
		    ROR		W68+1			;   6 3
ML01		BCC		ML02			;	4 2
			ADDB	W66+1			;   3 2
			ADCA	W66             ;   3 2
ML02		LSRA					;   2 1
			RORB					;   2 1
			ROR		W68+1			;	6 3
			DEX						;	4 1
			BNE     ML01            ;   4 2			28cycle/loop => *8=>224cycle
ML03		TST		W68				;	6 3
			BNE		ML031			;	4 2			Is W68,W68+1 <= 255?
			TBA						;	2 1
			BRA		ML07			;	4 2
ML031		LDX		#8				;	3 3
ML04	    ROR		W68				;   6 3
			BCC		ML05			;   4 2
			ADDB	W66+1			;   3 2
			ADCA	W66             ;   3 2
ML05		LSRA					;   2 1
			RORB					;   2 1
			DEX						;	4 1
			BNE     ML04            ;   4 2			↑28cycle/loop => *8=>224cycle
;;			ROR		W68				;   6 3
			LDAA	W68				;	3 2
			RORA					;   2 1
ML07		LDAB	W68+1			;	3 2
			RTS						;	5 1
*
*	MULTIPLY	AB = (W68+1) * (W66,W66+1)	when AccA==0
*
MULTI02		LDX		#8				;	3		before 3+2+2=7
;			CLRA					;	2		ここに来る時はAccAB=0
;			CLRB					;	2
ML201	    ASLB					;	2		loop 24cycle/loop -> 192cycle
			ROLA					;	2
			ROL     W68+1			;	6
			BCC     ML202			;	4
			ADDB	W66+1			;	3
			ADCA	W66             ;	3
ML202		DEX						;	4
			BNE     ML201           ;	4
			RTS
*
*
*	Division by powers of 2 (2^n)
*			AccAB		Number to be divided
*			SP+2,SP+3	Mask (2^n-1)			: not destroyed.
*			SP+4		Number of shifts (n)	: destroyed.
*
DIVPOW2		TSX
			STAB	W66+1
			STAA	W66
			BPL		DIVPOW01
			NEGA			; If the dividend is negative, make it positive
			NEGB
			SBCA	#0
DIVPOW01	ANDB	3,X		; mask low
			ANDA	2,X		; mask high
			STAB	_MOD+1
			STAA	_MOD
			LDAB	W66+1
			LDAA	W66
			BPL		DIVPOW02
			ADDB	3,X
			ADCA	2,X
DIVPOW02	ASRA				; 2 1
			RORB				; 2 1
			DEC		4,X			; 7 2
			BNE		DIVPOW02	; 4 2
DIVPOW99	RTS
*
*
ERROR		LDX		#DIV0ERROR
			JSR		PRINTSTR
			SWI
*
*	AB<=AB/(IX,IX+1),_MOD<=modulo
*
RDIVIDE		LDX		0,X
*
*	AB<=AB/IX,		 _MOD<=modulo
*
RDIVIDEX	STAA	W82+5
			BEQ		RDIVIDE01
			BPL		RDIVIDE02
			NEGA
			NEGB
			SBCA	#0
			BRA		RDIVIDE02
RDIVIDE01	TSTB
			BNE		RDIVIDE02
DIVZERO2	CLRB
			CLRA
			STAB	_MOD+1
			STAA	_MOD
			RTS
RDIVIDE02	STAB	W68+1
			STAA	W68
			STX		W66
			BEQ		ERROR
			BPL		RDIVIDE03
			COM		W82+5
			LDAA	W66
			NEGA
			NEG		W66+1
			SBCA	#0
			STAA	W66
RDIVIDE03	BSR     DIVPOS
			STAB    W4E+1		; modulo
			STAA    W4E
			LDAB    W68+1		; quotient
			LDAA    W68
			TST     W82+5
			BPL     RDIVIDE99
			NEGA
			NEGB
			SBCA	#0
RDIVIDE99	RTS
*
*	AB<=IX/AB,		_MOD<=modulo
*
DIVIDEX		CPX		#0
			BNE		DIVIDE00
DIVZERO		CLRB
			CLRA
			RTS
*
*
*	AB<=(IX,IX+1)/AB, _MOD<=modulo
*  (DIVPOS:W68<=W68/W66  AB<=Modulo)
*
*								; the dividend is often 0, it is more efficient to check it first.
DIVIDE		LDX		0,X			; If the dividend is 0, the answer is 0
			BEQ		DIVZERO
DIVIDE00	STAA	W82+5		; sign. bit7=0:plus, other:minus
			BMI		DIVIDE01	; AccA<0 -> AccAB<0
			BNE		DIVIDE02	; AccA>0 ?
			TSTB				; when AccA=0, AccB=0?
			BNE		DIVIDE02
			BRA		ERROR		; AccAB=0 -> Error
DIVIDE01	NEGA				; AccAB = abs(AccAB)
			NEGB
			SBCA	#0
DIVIDE02	STAB	W66+1
			STAA	W66
			STX		W68
			BPL     DIVIDE03
DIVIDE021	COM     W82+5		; if divient(W68)<0 then sign change
			LDAA	W68			; 3 2
			NEGA				; 2 1	; W68 = abs(W68)
			NEG		W68+1		; 6 3
			SBCA	#0			; 2 2
			STAA	W68			; 4 2	↑17cycle/10bytes
DIVIDE03	BSR     DIVPOS
			STAB    W4E+1		; modulo
			STAA    W4E
			LDAB    W68+1		; quotient
			LDAA    W68
			TST     W82+5
			BPL     DIVIDE99
			NEGA
			NEGB
			SBCA	#0
DIVIDE99	RTS
*
*	DIVIDE Positive number / non restoring division algorithm
*			W68 = W68/W66 AB=MODULO (W68=Q,W66=M,N=16,A=AccAB)
*			cf. https://www.javatpoint.com/non-restoring-division-algorithm-for-unsigned-integer
*			cf. https://www.wizforest.com/tech/Z80vs6502/div.html;p2
*			IX is destroyed
*
DIVPOS		EQU		*
			LDAB	W68			; 3 2	the dividend less than or equal to 255 (<=255?)
			BNE		DIVPOS00	; 4 2		↑7cycle overhead
			LDAA	W68+1		; 3 2	W68 = W68<<8
			CMPA	W66+1		; 3 2	W68<W66?
			BCC		DIVPOS11	; 4 2	may be No.
			TAB					; 2 1	AccAB = Modulo (=W68)
			CLRA				; 2 1
			STAA	W68+1		; 4 2	quotient = 0
			STAA	W68			; 4 2
			RTS					; 5 1
DIVPOS11	STAA	W68			; 3 2
			STAB	W68+1		; 3 2	clear W68+1
			LDX		#8-1		; 3 3
			BRA		DIVPOS10	; 4 2	now AccB=0, the jump destination is CLRA.
DIVPOS00	LDX		#16-1		; 3 3
			CLRB				; 2 1	clear AB
DIVPOS10	CLRA				; 2 1	7cycle for preparation
DIVPOS01	ROL		W68+1		; 6 3
			ROL		W68			; 6 3
			ROLB				; 2 1	shift AB<<1
			ROLA				; 2 1
			SUBB	W66+1		; 3 2
			SBCA	W66			; 3 2
			BCS		DIVMNS00	; 4 2
			SEC					; 2 1
			ROL		W68+1		; 6 3
			ROL		W68			; 6 3
DIVPLS01	ROLB				; 2 1
			ROLA				; 2 1
			SUBB	W66+1		; 3 2
			SBCA	W66			; 3 2
			BCS		DIVMNS02	; 4 2	↑14cycle
DIVPLS02	SEC					; 2 1
DIVPLS03	ROL		W68+1		; 6 3
			ROL		W68			; 6 3
			DEX					; 4 1
			BNE		DIVPLS01	; 4 2	↑22cycle	/ 36cycle for plus
			RTS					; 5 1
DIVMNS00	ASL		W68+1		; 6 3
			ROL		W68			; 6 3	↑12cycle
DIVMNS01	ROLB				; 2 1
			ROLA				; 2 1
			ADDB	W66+1		; 3 2
			ADCA	W66			; 3 2
			BCS		DIVPLS03	; 4 2	↑14cycle
DIVMNS02	ASL		W68+1		; 6 3
			ROL		W68			; 6 3
			DEX					; 4 1
			BNE		DIVMNS01	; 4 2	↑20cycle	/ 34cycle for minus
			ADDB	W66+1		; 3 2
			ADCA	W66			; 3 2
			RTS					; 5 1
*
*	signed/unsigned DIVIDE by 10
*			AB = AB/10, MOD=W4E
*			3 times faster than DIVPOS
*	cf. https://hackaday.com/2020/06/12/binary-math-tricks-shifting-to-divide-by-ten-aint-easy/
*	cf. http://homepage.divms.uiowa.edu/~jones/bcd/divide.html
*
			IF		IF_DIVS10
DIVS10		STAA	W66			; 4 2
			BPL		DIVU100		; 4 2
			NEGA				; 2 1
			NEGB				; 2 1
			SBCA	#0			; 3 2
			BSR		DIVU10		; 4 2
			NEGA				; 2 1
			NEGB				; 2 1
			SBCA	#0			; 3 2
			RTS					; 5 1
			ENDIF
			IF		IF_DIVU10+IF_DIVS10
DIVU10		STAA	W66			; 4 2
DIVU100		STAB	W66+1		; 4 2	save dividend n
			LSRA				; 2 1
			RORB				; 2 1
			STAB	W68+1		; 4 2	n>>1
			STAA	W68			; 4 2
			LSRA				; 2 1
			RORB				; 2 1	n>>2
			ADDB	W68+1		; 3 2
			ADCA	W68			; 3 2	q=(n>>1)+(n>>2)=n/2+n/4=3n/4
			STAB	W68+1		; 4 2
			STAA	W68			; 4 2
			LSRA				; 2 1
			RORB				; 2 1
			LSRA				; 2 1
			RORB				; 2 1
			LSRA				; 2 1
			RORB				; 2 1
			LSRA				; 2 1
			RORB				; 2 1	q>>4
			ADDB	W68+1		; 3 2
			ADCA	W68			; 3 2	q=q+(q>>4)=3n/4+3n/64=51n/64
			STAB	W68+1		; 4 2
			STAA	W68			; 4 2
			TAB					; 2 1
			CLRA				; 2 1	q>>8
			ADDB	W68+1		; 3 2
			ADCA	W68			; 3 2	q=q+(q>>8)=51n/64+51n/64/256=13107n/16384≒0.8n
			LSRA				; 2 1
			RORB				; 2 1
			LSRA				; 2 1
			RORB				; 2 1
			LSRA				; 2 1
			RORB				; 2 1	q>>3
			STAB	W68+1		; 4 2	
			STAA	W68			; 4 2	q=q>>3≒0.8n/8=n/10
			ASLB				; 2 1
			ROLA				; 2 1
			ASLB				; 2 1
			ROLA				; 2 1	q<<2
			ADDB	W68+1		; 3 2
			ADCA	W68			; 3 2	((q<<2)+q)
			ASLB				; 2 1
			ROLA				; 2 1	((q<<2)+q)<<1
			NEGA				; 2 1
			NEGB				; 2 1
			SBCA	#0			; 3 2
			ADDB	W66+1		; 3 2
			ADCA	W66			; 3 2
			STAB	W4E+1		; 4 2
			STAA	W4E			; 4 2
			SUBB	#10			; 2 2
			BCS		DIVU101		; 4 2	↑143
			STAB	W4E+1		; 4 2
			STAA	W4E			; 4 2
			LDAB	W68+1		; 3 2
			LDAA	W68			; 3 2
			ADDB	#1			; 3 2	; adjust answer by error term
			ADCA	#0			; 3 2
			RTS					; 5 1	↑168
DIVU101		LDAB	W68+1		; 3 2
			LDAA	W68			; 3 2
			RTS					; 5 1	↑152
			ENDIF
*
*	RANDOM 1 between AB
*			cf.https://tinyework.flston.com/xorshift-on-hd6301
*
			IF		IF_RANDOM
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
			LDX		W6A
			TAB
			CLRA
			JSR		MULTIPLYX
			TAB
			CLRA
			ADDB	#1
			ADCA	#0
			RTS
			ENDIF
*
* PRINT HEX sub. AccAB is destroyed
*
* PRINT 4 HEX AccAB
PRHEX4		PSHB
			TAB
			BSR		PRHEX2
			PULB
* PRINT 2 HEX AccB
PRHEX2		TBA
PR2HEX	    BSR		PRHEXH
			TBA
			ANDA    #$F
			BRA     PRHEXL
* PRINT 1 HEX AccA high nibble
PRHEXH		LSRA
			LSRA
			LSRA
			LSRA
* PRINT 1 HEX AccA low nibble
PRHEXL		ADDA	#$90		; 2 2
			DAA					; 2 1
			ADCA	#$40		; 2 2
PH1			DAA					; 2 1	8cycle/6bytes
			JMP		ASCOUT
*
* Print a space AccB times
*
PRINTTAB	TSTB
			BEQ     TABE
			LDAA	#' '
TAB1		JSR		ASCOUT
			DECB
			BNE     TAB1
TABE		RTS
*
*	PRINT AccAB decimal left
*
PRINTL		TSTA	#0
			BNE		PRINTL1
			CMPB	#10
			BCC		PRINTL1
PR1DEC		ADDB	#'0
			TBA
			JMP		ASCOUT
PRINTL1		BSR     BDSGN
PRINTSTR	LDAA    0,X				; 文字列が$00で始まってないこと(空で無いこと）
PRINTL2		JSR     ASCOUT
PRINTL3		INX
			LDAA	0,X
			BNE		PRINTL2
			RTS
*
*	PRINT AccAB decimal right width=WPRINTR
*
PRINTR		BSR		BDSGN
			STX		W64			; 
			LDAB	WPRINTR		; 右詰め桁数
			ADDB	W64+1		; 表示する最初の桁のアドレス下位
			SUBB	#W82+6		; バッファ最終桁+1のアドレス下位
			BMI		PRINTSTR
			BSR		PRINTTAB
PR1			BRA		PRINTSTR
*
*	符号付きDecimal作成
*	AccAB => (IX)
*
BDSGN		TSTA
			BPL		BYNDEC
BDNEG		NEGA
			NEGB
			SBCA	#0
			BSR		BYNDEC
			LDAB	#'-'
			DEX
			STAB	0,X
PRE			RTS
*
*	AccAB to Decimal W82+1..W82+5
*			return IX, it contains the first non-zero position (for zero blanking)
*
*	cf. https://homepage.cs.uiowa.edu/~jones/bcd/decimal.html
*
BYNDEC		LDX		#$2F3A			; #'0'-1,#'9'+1
			STX		W82+1
			STX		W82+3
BYNDEC1		INC		W82+1			; 6 3
			SUBB	#10000%256		; 2 2
			SBCA	#10000/256		; 2 2
			BCC		BYNDEC1			; 4 2		14cycle/loop
BYNDEC2		DEC		W82+2			; 6 3
			ADDB	#1000%256		; 2 2
			ADCA	#1000/256		; 2 2
			BCC		BYNDEC2			; 4 2		14cycle/loop
BYNDEC3		INC		W82+3			; 6 3
			SUBB	#100			; 2 2
			SBCA	#0				; 2 2
			BCC		BYNDEC3			; 4 2		14cycle/loop
			LDAA	#$3A			; 2 2
BYNDEC4		DECA					; 2 1
			ADDB	#10				; 2 2
			BCC		BYNDEC4			; 4 2		8cycle/loop + 2 + 4
			STAA	W82+4			; 4 2
			ADDB	#'0'
			COMB
			STAB	W82+5
ZEROBLK		LDX		#W82			; 変換終了。ゼロブランキングする
ZB1			INX						; Zero blanking
			LDAB	0,X
			CMPB	#'0'
			BEQ		ZB1
			COM		W82+5
ZB2			RTS
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
_TMP0		EQU		$00A3
_TMP1		EQU		$00A5
_TMP2		EQU		$00A7
_TMP3		EQU		$00A9
_TMP4		EQU		$00B1
_TMP5		EQU		$00B3
IXSAVE		EQU		$00B5		; Temporary IX WORK ; ADXWKと一緒でもいいか？
*
DBWK		RMB		2			; INPUT/DECBIN WORK
W4E			RMB		2
_MOD		EQU		W4E
W52			EQU		$00E0
RND			EQU		W52
W64			RMB		2			; PRINTR WORK
;W66		RMB		2
;W68		RMB		2
W66			EQU		$005D
W68			EQU		$006F
;W6A		RMB		2			; RANDOM WORK
W6A			EQU		$00AD
;W6C			RMB		2			; BYNDEC WORK
W6C			EQU		$00AF			; BYNDEC WORK
W6E			RMB		2			; BYNDEC WORK
W82			RMB		6			; DIVIDE BINDEC/WORK
			FCB		0
W88			RMB		1			;
WPRINTR		EQU		*			; PRINTR WORK 
_TMP6		RMB		2
_TMP7		RMB		2
_TMP8		RMB		2
_TMP9		RMB		2
_TMP10		RMB		2
_TMP11		RMB		2
_TMP12		RMB		2
_TMP13		RMB		2
_TMP14		RMB		2
_TMP15		RMB		2
PRGEND		EQU		*
* vim:set ts=4:
