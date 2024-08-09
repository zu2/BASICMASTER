all: GAME68.mot

%.bin: %.asm
	a09 -oM00 -L -dBASICROM=0 -dPRGBEG=\$$1000 -dTXTBEG=\$$3A00 -dDATETIME=`date +%s` -B$@ $<

%.mot: %.bin
	~/bin/objcopy --adjust-vma 0x1000 -I binary $< -O srec $@

GAME68.bin: GAME68.asm
GAME68.mot: GAME68.bin
