# BASICMASTER/電大版Tiny BASICの移植版

Softwares and Documents for Hitach BASIC MASTER L2/L2II/Jr.

## 電大版TinyBASICとは

電気通信大学で開発されたTiny BASIC。初出は「[マイ・コンピュータをつかう](https://amzn.to/3yoproY)」（講談社ブルーバックス 1978年4月25日）。後に[Bit誌1978年8月号](https://amzn.to/4bCmak8)に全ソースリストが掲載された。

## BASIC MASTERへの移植

大抵の言語処理系は0ページを山ほど使っているので、これをどこに移動するかが常に問題になります。
BASIC MASTERではモニタが00-71、BASICが72-FFを使っていてほとんど空きがありません。

モニタサブルーチンを利用するので、前半は使えず、後半のBASICの利用領域を騙し騙し使うしかありません。

別のページに移せばいいのですが、そうすると相対ジャンプで届かなくなる場所が発生して、玉突で修正量が大きくなります。少し遅くもなりますし。

この移植版ではL2 BASICの一時的な作業領域である$7A〜からを使うようにしています。

電大版TinyBASICは0ページの$82-$B5をA-Zの変数領域として使っています。$82はA($41)を2倍した値です。
さすがにこれだけの領域を使うのはもったいないので、この移植版では別ページにしています。が、今みたらバグってる気がする。TSTVはold MC6800さんのように変更した方が良いと思う。

- [DENDAI.srec](DENDAI.srec)
- [DENDAI.cmt](DENDAI.cmt)

## DENDAI.diffの説明

[MIKBUGプチ拡張 (Old MC6800)](https://oando.web.fc2.com/old_mc6800/f100_414.htm)に載っているTB2KD.ASMとの差分です。Bit誌との差分が理想的ですが再入力する元気はなかった。

- [DENDAI.diff](DENDAI.diff)


