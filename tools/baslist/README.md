# BASICMASTER / baslist

Softwares and Documents for Hitach BASIC MASTER L2/L2II/Jr.

## baslist

プリンタROMの代わりにこのプログラムを使うことで、LIST# コマンドの出力先をメモリに変更できる。

![baslist-1.png](./baslist-1.png)

現在のプログラムはBASICの裏にあるRAMに書き込むようになっているため、MB-6885でしか動かない。

また、書かれたデータを参照するためには $EFD0 に1を書いてRAMを選択する必要がある。戻す時は0を書く。
