all: GAME68.srec

%.bin: %.asm
	a09 -oM00 -L -dBASICROM=0 -dPRGBEG=\$$1000 -dTXTBEG=\$$3A00 -dDATETIME=`date +%s` $< 

%.srec: %.bin
	~/bin/objcopy --adjust-vma 0xB000 -I binary $< -O srec $@

GAME68.bin: GAME68.asm
GAME68.srec: GAME68.bin
