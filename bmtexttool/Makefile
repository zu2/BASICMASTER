all: txt2mot game2bin txt2bm ntb2bin bm2txt bin2bas txt2bmf

txt2mot:txt2mot.c
	gcc -O -o txt2mot txt2mot.c

game2bin:game2bin.c
	gcc -O -o game2bin game2bin.c

ntb2bin:ntb2bin.c
	gcc -O -o ntb2bin ntb2bin.c

txt2bm:txt2bm.c bmchar.h
	gcc -O -o txt2bm txt2bm.c

bm2txt:bm2txt.c bmchar.h
	gcc -O -o bm2txt bm2txt.c

txt2bmf:txt2bmf.c bmfullchar.h
	gcc -O -o txt2bmf txt2bmf.c

bin2bas:bin2bas.c
	gcc -O -o bin2bas bin2bas.c
