CFLAGS=-std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
.SUFFIXES: .hex .mot .game

gamecc: $(SRCS) common.h gamecc.asm
		$(CC) $(CFLAGS) -O -o gamecc $(SRCS) $(LDFLAGS)

$(OBJS): $(SRCS)

.game.hex: $(OBJS) gamecc
		./gamecc $< |tee $*.asm
		a09 -oM00 -X$*.hex -L$*.lst $*.asm

.game.mot: $(OBJS) gamecc
		./gamecc $< |tee $*.asm
		a09 -oM00 -S$*.mot -L$*.lst $*.asm

clean:
		rm -f gamecc *.o *~ tmp* *.hex *.mot

.PHONY: clean
