f9dasm: M6800/1/2/3/8/9 / H6309 Binary/OS9/FLEX9 Disassembler V1.83
Loaded binary file TinyGalaxy.bin

;****************************************************
;* Used Labels                                      *
;****************************************************

M0000   EQU     $0000
M011F   EQU     $011F
M0120   EQU     $0120
M013F   EQU     $013F
M017F   EQU     $017F
M037F   EQU     $037F
M0380   EQU     $0380
M039F   EQU     $039F
M03A0   EQU     $03A0
M101F   EQU     $101F

;****************************************************
;* Program Code / Data Areas                        *
;****************************************************

        ORG     $1000

        LDX     #M037F                   ;1000: CE 03 7F       '...'
Z1003   LDAA    ,X                       ;1003: A6 00          '..'
        CMPA    #$86                     ;1005: 81 86          '..'
        BNE     Z100F                    ;1007: 26 06          '&.'
        STAA    $20,X                    ;1009: A7 20          '. '
        LDAA    #$20                     ;100B: 86 20          '. '
        STAA    ,X                       ;100D: A7 00          '..'
Z100F   DEX                              ;100F: 09             '.'
        CPX     #M011F                   ;1010: 8C 01 1F       '...'
        BNE     Z1003                    ;1013: 26 EE          '&.'
        RTS                              ;1015: 39             '9'

        ORG     $1020 

        LDX     #M039F                   ;1020: CE 03 9F       '...'
Z1023   LDAA    ,X                       ;1023: A6 00          '..'
        CMPA    #$7C                     ;1025: 81 7C          '.|'
        BNE     Z102F                    ;1027: 26 06          '&.'
        STAA    $01,X                    ;1029: A7 01          '..'
        LDAA    #$20                     ;102B: 86 20          '. '
        STAA    ,X                       ;102D: A7 00          '..'
Z102F   DEX                              ;102F: 09             '.'
        CPX     #M011F                   ;1030: 8C 01 1F       '...'
        BNE     Z1023                    ;1033: 26 EE          '&.'
        RTS                              ;1035: 39             '9'

        ORG     $1040 

        LDX     #M0120                   ;1040: CE 01 20       '.. '
Z1043   LDAA    $01,X                    ;1043: A6 01          '..'
        CMPA    #$7B                     ;1045: 81 7B          '.{'
        BNE     Z104F                    ;1047: 26 06          '&.'
        STAA    ,X                       ;1049: A7 00          '..'
        LDAA    #$20                     ;104B: 86 20          '. '
        STAA    $01,X                    ;104D: A7 01          '..'
Z104F   INX                              ;104F: 08             '.'
        CPX     #M039F                   ;1050: 8C 03 9F       '...'
        BNE     Z1043                    ;1053: 26 EE          '&.'
        RTS                              ;1055: 39             '9'

        ORG     $1060 

Z1060   LDAA    M101F                    ;1060: B6 10 1F       '...'
        ASLA                             ;1063: 48             'H'
        ASLA                             ;1064: 48             'H'
        EORA    M101F                    ;1065: B8 10 1F       '...'
        INCA                             ;1068: 4C             'L'
        STAA    M101F                    ;1069: B7 10 1F       '...'
        RTS                              ;106C: 39             '9'

        ORG     $1080 

        LDX     #M037F                   ;1080: CE 03 7F       '...'
Z1083   LDAA    ,X                       ;1083: A6 00          '..'
        CMPA    #$20                     ;1085: 81 20          '. '
        BEQ     Z109E                    ;1087: 27 15          ''.'
        CMPA    #$86                     ;1089: 81 86          '..'
        BEQ     Z109E                    ;108B: 27 11          ''.'
        JSR     Z1060                    ;108D: BD 10 60       '..`'
        CMPA    M0000                    ;1090: 91 00          '..'
        BCC     Z109E                    ;1092: 24 0A          '$.'
        LDAA    $20,X                    ;1094: A6 20          '. '
        CMPA    #$86                     ;1096: 81 86          '..'
        BEQ     Z109E                    ;1098: 27 04          ''.'
        LDAA    #$86                     ;109A: 86 86          '..'
        STAA    $20,X                    ;109C: A7 20          '. '
Z109E   DEX                              ;109E: 09             '.'
        CPX     #M013F                   ;109F: 8C 01 3F       '..?'
        BNE     Z1083                    ;10A2: 26 DF          '&.'
        RTS                              ;10A4: 39             '9'

        ORG     $10C0 

        LDX     #M0380                   ;10C0: CE 03 80       '...'
Z10C3   LDAA    ,X                       ;10C3: A6 00          '..'
        CMPA    #$86                     ;10C5: 81 86          '..'
        BNE     Z10CD                    ;10C7: 26 04          '&.'
        LDAA    #$20                     ;10C9: 86 20          '. '
        STAA    ,X                       ;10CB: A7 00          '..'
Z10CD   INX                              ;10CD: 08             '.'
        CPX     #M03A0                   ;10CE: 8C 03 A0       '...'
        BNE     Z10C3                    ;10D1: 26 F0          '&.'
        RTS                              ;10D3: 39             '9'

        END
