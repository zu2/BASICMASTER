10 MUSIC T2P6ｿP3ｿﾗｼP5ｼﾗP3ﾌﾐP5ﾚP3ﾐﾚP5ｿP3ﾐﾚP7ﾄ
20 LET Z=$1000:LET Z1=$9E:MUSIC T1P0:CLEAR :RANDOMIZE :LET T=0:LET S=0
30 LET M=$3D0:POKE M,Z1
40 LET U1=1:LET U=$123:POKE U,7,5:LET CURSOR=0:PRINT "SCORE:";S
50 LET T=T+1:IF T=4 THEN LET T=0
60 IF PEEK(M)=Z1 THEN GOTO 140
70 IF PEEK(M)>7 THEN GOTO 120
80 LET S=S+INT(RND(5)+1)*50
90 FOR I=1 TO 10:POKE U,$9D:POKE U+1,$9D:MUSIC ｼ
100 POKE U,32:POKE U+1,32:MUSIC ｿ
110 NEXT I:POKE M+32,32:GOTO 30
120 LET CURSOR=10,10:PRINT "GAME OVER":FOR I=1 TO 10:POKE M,$2A:MUSIC ｿ
121 POKE M,32:MUSIC ｼ:NEXT I
130 POKE $82,0:POKE $83,0:RUN
140 POKE M,32:POKE M+32,32
150 LET A1=ASC(INKEY$):LET M=M-(A1=$56)+(A1=$4E)
160 IF (PEEK($EEC0)<>239)*(T/2<>INT(T/2)) THEN GOTO 180
170 LET M=M-$20:IF M<$120 THEN LET M=$3D0
180 IF (PEEK(M)<>$20)*(PEEK(M)<>Z1) THEN GOTO 70
190 POKE M,Z1:IF PEEK($EEC0)=239 THEN POKE M+32,$1A
200 IF T THEN GOTO 250
210 POKE U,32:POKE U+1,32
220 IF RND(10)<1 THEN LET U1=-U1
230 LET U=U+U1:IF (U<$124)+(U>$138) THEN LET U1=-U1
240 POKE U,7:POKE U+1,5
250 CALL Z:CALL Z+$C0:POKE 0,S/100:CALL Z+$20:CALL Z+$40:CALL Z+$80
260 IF RND(10)<1 THEN POKE $120+INT(RND(8))*64,$7C
270 IF RND(10)<1 THEN POKE $11F+INT(RND(8))*64,$7B
280 GOTO 50