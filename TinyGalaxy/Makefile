all:

disas:
	f9dasm -6800 -offset 1000 -noforced -info TinyGalaxy.info TinyGalaxy.bin > TinyGalaxy.asm

asm:
	a09 -oM00 TinyGalaxy.asm 

srec:
	~/bin/objcopy --adjust-vma 0x1000 -I binary TinyGalaxy.bin -O srec  TinyGalaxy.srec
