*** KUMAJIRI COMPILER ***
//" - TRANSFORM K-CODE -"//
A=$1FFE A=A(0) A:-4)=$FF
X=$1FFC X=X(0)
" SOURCE PROGRAM FROM $" ??=X+1
/" OBJECT PROGRAM FROM " Z=? Y=Z+3
 " VARIABLE ADDRESS FROM " V=?
N=0 I=$1600 D=$15C0 W=$1580 F=$1540 S=$1500 T=$1000
Z(0)=$BD0B Z:2)=0
T:0)=$FF L=$1600 @
N=N+1 X=X+1 / ?(7)=N "  $" ??=X "  $" ??=Y @
 &=X:0)=" " X=X+1 & ;=X:0)=$D #=NEXT;
 ;=X:0)="*" @ X=X+1 @=X:0)=$D #=NEXT;
 ;=X:0)="/" Y:0)=40 Y=Y+1 #=NEXT2;
 ;=X:0)=""" Y:0)=39 @
  Y=Y+1 X=X+1 Y:0)=X:0) @=X:0)="""
  Y:-1)=Y:-1)+$80 #=NEXT2;
 ;=X:0)>="A"&(X:0)<="Z")
  !=TSEARCH ;=X:0)=".".(X:0)=" ") #=LABEL;
  ;=C=3 T:0)=1 T(1)=V V=V+2 :
   ;=T:0)<>1 #=SERROR;; H=T(1)
  ;=X:0)=":" M=14 #=L450;
  ;=X:0)="(" M=15 L450. X=X+1 S=S-1 S:0)=0 !=EVEXP S=S+1
   X=X+1 !=EVEX Y:0)=M #=L500;
  X=X+1 !=EVEXP ;=X:0)<>"," Y:0)=13 L500. Y=Y+1 Y(0)=H
  Y=Y+2 #=NEXT;
  X=X+1 !=EVEX Y(0)=$405 Y(1)=H F=F-2 F(0)=Y+4 Y=Y+6 #=NEXT;
 ;=X:0)="$" M=45 #=L780;
 ;=X:0)="?"
 ;=X:1)="?" X=X+1 M=43 #=L780;
 ;=X:1)="$" X=X+1 M=44 #=L780;
 ;=X:1)="("
  X=X+2 S=S-1 S:0)=0 !=EVEXP S=S+1 X=X+1 !=EVEX Y:0)=42
  Y=Y+1 #=NEXT;
 M=41 L780. X=X+2 G=0 !=EVEX Y:0)=M Y=Y+1 #=NEXT;
 ;=X:0)="\"
  ;=F=$1540 /"NEXT WITHOUT FOR."/ #=ERROR;
  Y:0)=0 Y=Y+1 Y(0)=F(0)-3 Y=Y+2 A=F(0) F=F+2 A(0)=Y #=NEXT2;
 ;=X:0)="@"
  ;=X:1)="="
   ;=D=$15C0 /"UNTIL WITHOUT DO."/ #=ERROR;
   X=X+2 !=EVEXP Y:0)=3 !=R Y=Y+1 Y(0)=D(0) D=D+2 Y=Y+2 #=NEXT;
  D=D-2 D(0)=Y #=NEXT2;
 ;=X:0)="&"
  ;=X:1)="="
   W=W-4 W(1)=Y X=X+2 !=EVEXP Y:0)=3 !=R W(0)=Y+1 Y=Y+3 #=NEXT;
  ;=W=$1580 /" CONTINUE WITHOUT WHILE."/ #=ERROR;
  Y:0)=0 Y=Y+1 Y(0)=W(1) A=W(0) Y=Y+3 W=W+4 A(0)=Y #=NEXT2;
 ;=X:0)=";"
  ;=X:1)="="
   I=I-2 X=X+2 !=EVEXP Y:0)=3 !=R I(0)=Y+1 Y=Y+3 #=NEXT;
  ;=I=$1600 /" IF-END WITHOUT IF."/ #=ERROR;
  A=I(0) A(0)=Y I=I+2 #=NEXT2;
 ;=X:0)=":"
  ;=I=$1600 /" ELSE WITHOUT IF."/ #=ERROR;
  A=I(0) Y:0)=0 I(0)=Y+1 Y=Y+3 A(0)=Y #=NEXT2;
 ;=X:0)="]" Y:0)=2 Y=Y+1 #=NEXT2;
 ;=X:0)="#" Y:0)=0 #=L1300;
 ;=X:0)="!" Y:0)=1 L1300.
  X=X+2 !=TSEARCH Y=Y+1
  ;=C=3 Y(0)=0 T(0)=1 T(1)=Y Y=Y+2 #=NEXT;
  ;=T:0) #=SERROR;
  Y(0)=T(1) Y=Y+2 ;=T:1)=0 #=NEXT;
  T(1)=Y-2 #=NEXT;
 ;=X:0)=">"&(X:1)="=")
  X=X+2 !=EVEXP Y:0)=51 Y=Y+1 #=NEXT;
 ;=X:0)="."&(X:1)="=")
  X=X+2 !=EVEXP Y:0)=46 Y=Y+1 #=NEXT;
#=SERROR
LABEL.
 ;=C=3 T(0)=0 T(1)=Y #=NEXT2;
 ;=T:0) #=ERROR;
 ;=T:1)=0 /" DOUBLE DEFINED ERROR."/ #=ERROR;
 B=T(1) T:1)=0 @
  A=B(0) B(0)=Y B=A
 @=B=0 T(1)=Y #=NEXT2
*
TSEARCH.
 C=0,3 L(C)=$2020 \ C=0 @
  L:C)=X:0) C=C+1 X=X+1
 B=X:0) !=LORN @=B=0 C=0 T=$1000 @
 ;=T:0)=$FF C=3 E=0,3 T(2+E)=L(E) \ T:12)=$FF :
  E=0 &=L:E)=T:E+4)&(E<8) E=E+1 &
  ;=E=8 C=1 : T=T+12 ;;
 @=C>0 ]
*
R. ;=Y:-1)>23&(Y:-1)<30)&(Q=1) Y=Y-1 Y:0)=Y:0)+9 ;]
*
EVEXP. G=0 EVEX. U=0 EVLOOP.
;=X:0)=$D.(X:0)=",").(X:0)=" ")
 ;=U=0 #=SERROR;
 ;=S=$1500 Q=0 ];
 @ ;=S:0)<2 /" THERE ISN'T CLOSE PARENTHESIS."/ #=ERROR;
  Y:0)=S:0) Y=Y+1 S=S+1
 @=S=$1500 Q=1 ];
;=X:0)>="0"&(X:0)<="9")
 O=0 @ O=O*10+X:0)-"0" X=X+1 @=X:0)<"0".(X:0)>"9") #=L5110;
;=X:0)=""" O=X:1) X=X+3 #=L5110;
;=X:0)="$"
 ;=X:1)>"9"&(X:1)<"A").(X:1)<"0").(X:1)>"F") #=L5760;
 O=0 A=0 @ ;=A>9 A=A-7 ; O=O*16+A X=X+1 A=X:0)-"0"
@=A>9&(A<17).(A<0).(A>22)
 L5110. ;=S:0)=17&(S<$1500) S=S+1 O=-O ;
 C=8 ;=G=0 G=1 C=7 ; #=L5280;
;=X:0)>="A"&(X:0)<="Z")
 !=TSEARCH ;=C=3 T:0)=1 T(1)=V V=V+2 : ;=T:0)<>1 #=SERROR;; O=T(1)
 ;=X:0)=":" C=11 #=L5250;
 ;=X:0)="(" C=12 L5250.
  ;=U=1 #=SERROR;
  S=S-4 S:0)=0 S:1)=C S(1)=O X=X+1 !=EVEX C=S:1) O=S(1) S=S+4
  U=0 #=L5280;
C=10 ;=G=0 G=1 C=9 ;
L5280. ;=U=1 #=SERROR; U=1
Y:0)=C Y=Y+1 Y(0)=O Y=Y+2 #=EVLOOP;
;=X:0)="("
 ;=U=1 #=SERROR;
 S=S-1 S:0)=1 X=X+1 #=EVEX;
;=X:0)=")"
 ;=U=0 #=SERROR;
 X=X+1 &=S:0)<>1
  ;=S:0)=0 ];
  ;=S=$1500 /" THERE ISN'T OPEN PARENTHESIS."/ #=ERROR;
 Y:0)=S:0) Y=Y+1 S=S+1
 & S=S+1 U=1 #=EVLOOP;
;=X:0)="#" C=16 #=N2;
;=X:0)="'" C=19 N2. ;=U=1 #=SERROR; U=1 X=X+1 #=N3;
;=X:0)="+" ;=U=0 C=18 #=N2 : C=22 ; #=L562;
;=X:0)="-" ;=U=0 C=17 #=N2 : C=23 ; #=L562;
;=X:0)="*" C=20 #=L562;
;=X:0)="/" C=21 #=L562;
;=X:0)="=" C=28 #=L562;
;=X:0)="&" C=30 #=L562;
;=X:0)="." C=31 #=L562;
;=X:0)="!" C=32 #=L562;
;=X:0)=">"
 ;=X:1)="=" X=X+1 C=25 : C=24 ; #=L562;
;=X:0)="<"
 ;=X:1)="=" X=X+1 C=27 #=L562;
 ;=X:1)=">" X=X+1 C=29 : C=26 ;
L562. ;=U=0 #=SERROR;
X=X+1 &=S<$1500&(S:0)>1)
 Y:0)=S:0) Y=Y+1 S=S+1
& N3. S=S-1 S:0)=C #=EVEX;
;=X:0)="?"
 X=X+1 C=49 ;=G=0 G=1 C=50 ; #=L577
 L5760. X=X+1 C=47 ;=G=0 G=1 C=48 ;
L577. ;=U=1 #=SERROR; U=1
Y:0)=C Y=Y+1 #=EVLOOP;
SERROR.
 /" SYNTAX ERROR."/ #=ERROR
*
NEXT2. X=X+1 NEXT. @=X:0)=$D
@=X:1)=$FF
EFLAG=0
;=I<$1600 !=IL "IF." ;
;=D<$15C0 !=IL "DO." ;
;=W<$1580 !=IL "WHILE." ;
;=F<$1540 !=IL "FOR." ;
*
/ T=$1000 U=0 &=T:0)<$FF
;=T(0)=1
 ;=U=0 U=1 /"UNDEFINED LABELS :"/;
 I=4,11 $=T:I) \; T=T+12
&
/" VARIABLE'S ADDRESS :"/
T=$1000 &=T:0)<$FF
 ;=T:0)=1 I=4,11 $=T:I) \ " $" ??=T(1) "  " ; T=T+12
&
//" TRANSFORMATION END."
/"   OBJECT PROGRAM : $" ??=Z "-$" ??=Y+1 Y(1)=$776 Y(2)=$A033
>=$1FA0
IL. /" ILLEGAL NESTING " EFLAG=1 ]
LORN. ;=B>"9"&(B<"A").(B<"0").(B>"Z") B=0 : B=1 ; ]
ERROR. A=X @ A=A-1 @=A:0)=$D B=$A3 B(0)=N
@ A=A+1 $=A:0) @=A=X / >=$1FA5
*
****
