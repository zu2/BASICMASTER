all: NTB.mot NTB.hex

%.bin: %.asm
	a09 -oM00 -L -dDATETIME=`date +%s` -dPRGBEG=\$$2000 -dBASICROM=1 $< 
	mv -f $*.b $@

%.mot: %.bin
	objcopy --adjust-vma 0x2000 -I binary $< -O srec $@

%.hex: %.bin
	objcopy --adjust-vma 0x2000 -I binary $< -O ihex $@

NTB.bin: NTB.asm
NTB.mot: NTB.bin
NTB.hex: NTB.bin
