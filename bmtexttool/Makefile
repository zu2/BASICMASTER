CFLAGS=-O -Wno-pointer-sign

all: txt2mot game2bin txt2bm ntb2bin bm2txt bin2bas txt2bmf game2txt

txt2mot:txt2mot.c
	gcc $(CFLAGS) -o txt2mot txt2mot.c

game2bin:game2bin.c
	gcc $(CFLAGS) -o game2bin game2bin.c

ntb2bin:ntb2bin.c
	gcc $(CFLAGS) -o ntb2bin ntb2bin.c

txt2bm:txt2bm.c bmchar.h
	gcc $(CFLAGS) -o txt2bm txt2bm.c

bm2txt:bm2txt.c bmchar.h
	gcc $(CFLAGS) -o bm2txt bm2txt.c

txt2bmf:txt2bmf.c bmfullchar.h
	gcc $(CFLAGS) -o txt2bmf txt2bmf.c

bin2bas:bin2bas.c
	gcc $(CFLAGS) -o bin2bas bin2bas.c

game2txt:game2txt.c
	gcc $(CFLAGS) -o game2txt game2txt.c
