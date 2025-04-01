# CHECKSUM program

There are two checksum methods: MLPT and MLW.
This program uses the MLW method.

The MLW method simply adds up each byte.

The MLPT method adds up the 2's complement of each byte.

# Usage

- CALL $6000
- if you want to print out the dump list, type 'Y'. otherwise type 'N'
- type start address
- push space bar to continue.

# relocate binary

change address for JMP, LDX #

- $603B, $609A			JSR, JMP
- $6000, $6035, $607A	LDX #

# demo

[!checksum-dump.png](./checksum-dump.png)


# reference

- BASIC MASTER USER'S GROUP NO.5 MAY 1982 P.7-P.8
	
