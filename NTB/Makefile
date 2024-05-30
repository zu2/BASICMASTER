all: NTB.srec NTB.hex

#bin.asm:
#	f9dasm -6800 -offset 1000 -noforced -info $*.info $< > $@

#asm.bin:
#	a09 -oM00 $< 

#bin.srec:
#	~/bin/objcopy --adjust-vma 0x1000 -I binary $< -O srec $@

%.bin: %.asm
	a09 -oM00 -L -dDATETIME=`date +%s` -dOBJSTART=$B000 -dBASICROM=1 $< 
	mv -f $*.b $@

%.srec: %.bin
	~/bin/objcopy --adjust-vma 0xB000 -I binary $< -O srec $@

%.hex: %.bin
	~/bin/bin2hex $< $@ 0xB000 

NTB.bin: NTB.asm
NTB.srec: NTB.bin

pbcopy:
	~/basicmaster/TB2KD110/bin2dump <NTB.bin |pbcopy
