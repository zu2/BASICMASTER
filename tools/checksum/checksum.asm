*
* BM版タテ-ヨコ チェックサム
*				by TEW SOFT
*			(ZUKERAN,shin. @zu2)
*
* BASIC MASTER USER'S GROUP
*  No.5 MAY 1982
*
* There are two checksum methods: MLPT and MLW. This program uses the MLW method.
*   The MLW  method simply adds up each byte.
*   The MLPT method adds up 2's complement of each byte.
*
BREAKV  EQU     $0004
ASCIN   EQU     $0028
ASCOUT  EQU     $002B
M002C   EQU     $002C
ADRTOP  EQU     $003B
FNAME   EQU     $0043
; file attribute
filetype EQU     $004B                    ; B:01, S:10
; blocknumber (BASIC save blocks)
block_num EQU     $004C
; alter filename for compare/verify
FNAME2  EQU     $004D
ZE000   EQU     $E000
ME10F   EQU     $E10F
CHRGET  EQU     $F012
; display 1 char. 01-0f are control code. when char is control code, return CARRY=1
CHROUT  EQU     $F015
; hex output byte data from IX
HEX2OUT_from_IX EQU     $F21D
; HEX4IN to (IX)
HEX4IN_ADRTOP EQU     $F3AC
; display IX in hex
OUTIX   EQU     $FFEC
; display message from (IX), terminate with 04
MESOUT  EQU     $FFF2

;****************************************************
;* Program Code / Data Areas                        *
;****************************************************

        ORG     $6000

        LDX     #mPRINTER                ;6000: CE 60 C6       '.`.'
        JSR     MESOUT                   ;6003: BD FF F2       '...'
        CLR     >block_num               ;6006: 7F 00 4C       '..L'
        LDAA    #$9D                     ;6009: 86 9D          '..'
        JSR     >ASCIN                   ;600B: BD 00 28       '..('
        CMPA    #$59                     ;600E: 81 59          '.Y'
        BNE     Z6018                    ;6010: 26 06          '&.'
        COM     >block_num               ;6012: 73 00 4C       's.L'
        JSR     ZE000                    ;6015: BD E0 00       '...'
Z6018   JSR     HEX4IN_ADRTOP            ;6018: BD F3 AC       '...'
        LDAA    #$0C                     ;601B: 86 0C          '..'
        JSR     >ASCOUT                  ;601D: BD 00 2B       '..+'
        TST     >block_num               ;6020: 7D 00 4C       '}.L'
        BEQ     Z602A                    ;6023: 27 05          ''.'
        LDX     #ME10F                   ;6025: CE E1 0F       '...'
        STX     M002C                    ;6028: DF 2C          '.,'
Z602A   LDAB    #$08                     ;602A: C6 08          '..'
        LDX     #FNAME                   ;602C: CE 00 43       '..C'
Z602F   CLR     ,X                       ;602F: 6F 00          'o.'
        INX                              ;6031: 08             '.'
        DECB                             ;6032: 5A             'Z'
        BNE     Z602F                    ;6033: 26 FA          '&.'
        LDX     #mADRS                   ;6035: CE 60 D0       '.`.'
        JSR     MESOUT                   ;6038: BD FF F2       '...'
        JSR     Z60BB                    ;603B: BD 60 BB       '.`.'
        LDAB    #$10                     ;603E: C6 10          '..'
Z6040   PSHB                             ;6040: 37             '7'
        LDX     ADRTOP                   ;6041: DE 3B          '.;'
        JSR     OUTIX                    ;6043: BD FF EC       '...'
        LDX     #FNAME                   ;6046: CE 00 43       '..C'
        STX     FNAME2                   ;6049: DF 4D          '.M'
        CLRB                             ;604B: 5F             '_'
        LDAA    #$08                     ;604C: 86 08          '..'
Z604E   PSHA                             ;604E: 36             '6'
        LDX     ADRTOP                   ;604F: DE 3B          '.;'
        ADDB    ,X                       ;6051: EB 00          '..'
        LDAA    ,X                       ;6053: A6 00          '..'
        LDX     FNAME2                   ;6055: DE 4D          '.M'
        ADDA    ,X                       ;6057: AB 00          '..'
        STAA    ,X                       ;6059: A7 00          '..'
        INX                              ;605B: 08             '.'
        STX     FNAME2                   ;605C: DF 4D          '.M'
        LDX     ADRTOP                   ;605E: DE 3B          '.;'
        BSR     Z60A6                    ;6060: 8D 44          '.D'
        STX     ADRTOP                   ;6062: DF 3B          '.;'
        PULA                             ;6064: 32             '2'
        DECA                             ;6065: 4A             'J'
        BNE     Z604E                    ;6066: 26 E6          '&.'
        BSR     Z60AE                    ;6068: 8D 44          '.D'
        PULB                             ;606A: 33             '3'
        DECB                             ;606B: 5A             'Z'
        BNE     Z6040                    ;606C: 26 D2          '&.'
        LDAA    #$2D                     ;606E: 86 2D          '.-'
        LDAB    #$20                     ;6070: C6 20          '. '
Z6072   JSR     >ASCOUT                  ;6072: BD 00 2B       '..+'
        DECB                             ;6075: 5A             'Z'
        BNE     Z6072                    ;6076: 26 FA          '&.'
        BSR     Z60BB                    ;6078: 8D 41          '.A'
        LDX     #mSUM                    ;607A: CE 60 F1       '.`.'
        JSR     MESOUT                   ;607D: BD FF F2       '...'
        LDX     #FNAME                   ;6080: CE 00 43       '..C'
        CLRB                             ;6083: 5F             '_'
        LDAA    #$08                     ;6084: 86 08          '..'
Z6086   PSHA                             ;6086: 36             '6'
        ADDB    ,X                       ;6087: EB 00          '..'
        BSR     Z60A6                    ;6089: 8D 1B          '..'
        PULA                             ;608B: 32             '2'
        DECA                             ;608C: 4A             'J'
        BNE     Z6086                    ;608D: 26 F7          '&.'
        BSR     Z60AE                    ;608F: 8D 1D          '..'
        BSR     Z60C0                    ;6091: 8D 2D          '.-'
        JSR     CHRGET                   ;6093: BD F0 12       '...'
        CMPA    #$20                     ;6096: 81 20          '. '
        BNE     Z609D                    ;6098: 26 03          '&.'
        JMP     Z602A                    ;609A: 7E 60 2A       '~`*'
Z609D   LDX     #CHROUT                  ;609D: CE F0 15       '...'
        STX     M002C                    ;60A0: DF 2C          '.,'
        LDX     BREAKV                   ;60A2: DE 04          '..'
        JMP     ,X                       ;60A4: 6E 00          'n.'
Z60A6   JSR     HEX2OUT_from_IX          ;60A6: BD F2 1D       '...'
        LDAA    #$20                     ;60A9: 86 20          '. '
        JMP     >ASCOUT                  ;60AB: 7E 00 2B       '~.+'
Z60AE   LDAA    #$3A                     ;60AE: 86 3A          '.:'
        JSR     >ASCOUT                  ;60B0: BD 00 2B       '..+'
        STAB    filetype                 ;60B3: D7 4B          '.K'
        LDX     #filetype                ;60B5: CE 00 4B       '..K'
        JSR     HEX2OUT_from_IX          ;60B8: BD F2 1D       '...'
Z60BB   TST     >block_num               ;60BB: 7D 00 4C       '}.L'
        BEQ     Z60C5                    ;60BE: 27 05          ''.'
Z60C0   LDAA    #$0D                     ;60C0: 86 0D          '..'
        JSR     >ASCOUT                  ;60C2: BD 00 2B       '..+'
Z60C5   RTS                              ;60C5: 39             '9'
mPRINTER FCB     $0C                      ;60C6: 0C             '.'
        FCC     "PRINTER?"               ;60C7: 50 52 49 4E 54 45 52 3F 'PRINTER?'
        FCB     $04                      ;60CF: 04             '.'
mADRS   FCC     "ADRS +0 +1 +2 +3 +4 +"  ;60D0: 41 44 52 53 20 2B 30 20 2B 31 20 2B 32 20 2B 33 20 2B 34 20 2B 'ADRS +0 +1 +2 +3 +4 +'
        FCC     "5 +6 +7 SUM"            ;60E5: 35 20 2B 36 20 2B 37 20 53 55 4D '5 +6 +7 SUM'
        FCB     $04                      ;60F0: 04             '.'
mSUM    FCC     "SUM: "                  ;60F1: 53 55 4D 3A 20 'SUM: '
        FCB     $04                      ;60F6: 04             '.'
        END
