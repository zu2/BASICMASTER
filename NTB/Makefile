all: NTB.srec NTB.hex

%.bin: %.asm
	a09 -oM00 -L -dDATETIME=`date +%s` -dPRGBEG=\$$2000 -dBASICROM=1 $< 
	mv -f $*.b $@

%.srec: %.bin
	objcopy --adjust-vma 0x2000 -I binary $< -O srec $@

%.hex: %.bin
	bin2hex $< $@ 0x2000 

NTB.bin: NTB.asm
NTB.srec: NTB.bin
