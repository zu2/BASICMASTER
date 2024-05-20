all: baslist.srec

%.bin: %.asm
	a09 -oM00 -L $< 
	mv -f $*.b $@

%.srec: %.bin
	~/bin/objcopy --adjust-vma 0x1000 -I binary $< -O srec $@

GAME.bin: baslist.asm
GAME.srec: baslist.bin

pbcopy:
		~/basicmaster/TB2KD110/bin2dump <baslist.bin |pbcopy
