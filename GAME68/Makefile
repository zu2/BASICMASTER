all: GAME68.srec

%.bin: %.asm
	a09 -oM00 -L -dORG=\$$B000 -dBASICROM=1 -dPRGBEG=\$$1000 -dDATETIME=`date +%s` $< 

%.srec: %.bin
	~/bin/objcopy --adjust-vma 0xB000 -I binary $< -O srec $@

GAME68.bin: GAME68.asm
GAME68.srec: GAME68.bin
