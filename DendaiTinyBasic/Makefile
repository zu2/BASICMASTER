all: DENDAI.mot

%.bin: %.asm
	a09 -oM00 -L $< 
	mv -f $*.b $@

%.mot: %.bin
	objcopy --adjust-vma 0x1000 -I binary $< -O srec $@

DENDAI.bin: DENDAI.asm
DENDAI.srec: DENDAI.bin

DENDAI.asm: TB2KD.ASM
	patch -p1 TB2KD.ASM -o DENDAI.asm < DENDAI.diff

TB2KD.ASM: TB2KD110.zip
	unzip TB2KD110.zip TB2KD.ASM

TB2KD110.zip:
	curl -O https://oando.web.fc2.com/old_mc6800/download/TB2KD110.zip
