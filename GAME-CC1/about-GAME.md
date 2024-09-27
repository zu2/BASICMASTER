# GAME language

GAME (Geneal Algorithmic Micro Expression) is a programming language with a grammar similar to VTL (Very Tiny Language). It has functions similar to Tiny BASIC.

The first interpreter released for the 6800 was 1.5KB in size, and 4KB of memory was enough to run it.

There were 26 variables from A to Z, but each variable could be used as a pointer, allowing easy 1-byte or 2-byte access. Therefore, as the name suggests, GAME was used for game development and system development. A self-compiler for the GAME language was also created using the GAME language, demonstrating its high descriptive capabilities.

## Development and porting to various PCs

GAME was developed by Hiroshi Onishi and published in the Japanese magazine ASCII on page 66 of the July 1978 issue.

The first version by Onishi was for the Japanese microcomputer kit H68/TR. The GAME language was accepted by Japanese computer users at the time and was published for many models.

The programs that have been released are listed in the table below, but they have also been ported to Hitachi Basic Master L2/Jr, 8086/MS-DOS/Windows, Linux, etc.

### Interpreter

+-----------+-----------+-------+
|Name		|System		|CPU	|
+-----------+-----------+-------+
|GAME		|H68/TR		|MC6800 |
|GAME68		|H68/TR+TV	|MC6800 |
|GAME80		|TK-80BS	|i8080	|
|GAME-Z80	|TRS-80		|Z80	|
|GAME-MZ	|MZ-80		|Z80	|
|GAME65		|PET		|6502	|
|GAME-APPLE	|APPLE II	|6502	|
|GAME-PC	|PC-8001	|Z80	|
|GAME09x	|			|6809	|
|GAME-L3	|BM Level3	|6809	|
|GAME-SMC	|SMC-70		|Z80	|
|GAME-FM	|FM-7/8/11	|6809	|
+-----------+-----------+-------+

### Compiler

+-----------------------+-----------+-------+
|Name |System |CPU |
+-----------------------+-----------+-------+
|GAME80 Compiler |TK-80BS |i8080 |
|GAME-MZ Compiler |MZ-80 |Z80 |
|GAME-APPLE Compiler |APPLE II |6502 |
|GAME-PC Compiler |PC-8001 |Z80 |
|GAME68 Compiler |H68/TR |MC6800 |
|GAME-FM Compiler |FM-7/8/11 |MC6809 |
+-----------------------+-----------+-------+

- , General Algorithimic Micro Expression (TBN),ASCII, 1978(5)
- GAME A new language for microcomputers, ASCII, 1978(7), p.66
- GAME Part 2: Research on interpreters, ASCII, 1978(8), p.42
- GAME Part 3: Line editing and discompilers, ASCII, 1978(9), p.68
- GAME Part 4: Assembler for the 6800, ASCII, 1978(10),p.66
- GAME68 on H68/TR+TV (TBN), ASCII, 1978(11), p.62
- GAME80, ASCII, 1979(1), p.38
- Extensions of GAME, ASCII, 1979(1), p.61
- GAME-Z80, ASCII, 1979(4), p.40
- GAME80 Compiler, ASCII, 1979(7), p.26
- Utility Programs for GAME68, ASCII, 1979(8), p.71
- GAME65, ASCII, 1979(10), p.24
- GAME-MZ, ASCII, 1979(10), p.30
- GAME-MZ Compiler, ASCII, 1979(10), p.34
- GAME-APPLE, ASCII, 1980(2), p.68
- GAME-APPLE Optimizing Compiler, ASCII, 1980(4), p.86
- GAME-APPLE plus Compiler, ASCII, 1980(6), p.79
- GAME-PC, ASCII, 1980(9), p.72
- GAME-PC Compiler, ASCII, 1980(10), p.142
- GAME68 Compiler, ASCII, 1981(5), p.149
- Introduction to GAME, ASCII, 1981(6), p.87
- GAME09 Interpreter, 1981(6), p.90
- GAME09, ASCII, 1981(6), p.90
- GAME-L3, ASCII, 1981(6), p.94
- GAME-SMC, ASCII, 1983(6), p.95
- GAME-FM, ASCII, 1984(9), p.205

# Reference

- [snakajima/game-compiler: Game 80 compiler, which I've created when I was 18 ('79)](https://github.com/snakajima/game-compiler)
- [GAME68 compiler: I tried electronic crafting](https://telmic.exblog.jp/30174191/)
- [GAME80 Compiler](https://www.mztn.org/rvtl/game80.html)
- [No. 33: The value of "reinventing the wheel" | gihyo.jp](https://gihyo.jp/lifestyle/serial/01/software_is_beautiful/0033)
- [Retro PC Gallery](http://haserin09.la.coocan.jp/proglangdevsys.html)
- [GAME86 Compiler for MS-DOS](https://www.mztn.org/game86/)
- [GAME APPLE II](http://ohta.html.xdomain.jp/mac/gameapple.html)
- [History of VTL-based languages](https://www.mztn.org/rvtl/vtltabl.html)
- [I tried making a GAME language interpreter | g200kg Music & Software](https://www.g200kg.com/archives/2022/12/game.html)
- [GAME68 Compiler: I tried some electronics work](https://telmic.exblog.jp/30174191/)
- [GAME Language Interpreter in C [GAMEC] #C - Qiita](https://qiita.com/fygar256/items/7b90aadfe5a80cb193f0)

