# MB-6885 mainboard memo

## Jumper

- JP1,2,3
	- 64K RAM expansion jumper
	- 16K: 1-2 short
	- 64K: 2-3 short
- JP4,5
	- MP-3550, MP-3530 switch
	- 1-3 short (MP-3550, 5.25inch 2D 2drive)
	- 1-2 short (MP-3530, 5.25inch 1S 2drive)
- JP6
	- IC78 μPD2316EC, 2717 switch
	- 1-3 short (2316, 2Kx8bit mask ROM)
	- 1-2 2717?


## Unpopulated IC footprint

- IC97,98,99
	- 74LS367 DMA
	- Implemented for internal memory access via expansion port
- 14-pin and 16-pin unpopulated IC footprints near IC39
	- Pins are unconnected (NC); IC type and function unknown.

