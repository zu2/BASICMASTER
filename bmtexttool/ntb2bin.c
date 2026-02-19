//
//	text file を NTBの格納形式に変換する
//	GAMEとほぼ同じだが、行番号直後のスペースは無くても良い。
//
//		[行番号2バイト][テキスト内容][00]....[FF]
//
//			by ZUKERAN, shin
//		(@zu2, aka TEW SOFT)
//
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<string.h>
#include	<unistd.h>
#include	<ctype.h>

extern	int
main(int argc, char **argv)
{
	if(argc<2) {
		fprintf(stderr,"usage: %s txtfile\n",argv[0]);
		exit(1);
	}
	char	*filename = argv[1];
	char	*e;
	int		fd = open(filename,O_RDONLY);
	if(fd<0){
		perror(NULL);
		fprintf(stderr,"can't open %s\n",filename);
		exit(1);
	}
	struct	stat	stbuf;
	if(fstat(fd,&stbuf)<0) {
		perror("can't stat");
		exit(1);
	}
	long filesize = stbuf.st_size;
	unsigned char	*buffer;
	if((buffer=(unsigned char *)malloc(filesize+1))==NULL){
		perror("can't malloc");
		exit(1);
	}
	read(fd,buffer,filesize);
	buffer[filesize]=0;

	unsigned char	*p = buffer;
	while(*p && *p=='\n'){		// 先頭の空行を読み飛ばす
		p++;
	}
	while(*p){
		unsigned int	line_no = 0;
		unsigned char	ch;
		while((ch=*p) && isdigit(ch)){
			line_no = line_no*10+(ch-'0');
			p++;
		}
		putchar((line_no&0x0ff00)>>8);
		putchar(line_no&0x0ff);
		if((ch=*p) && (ch==' ')){		// 行番号直後のスペースを取る
			p++;
		}
		while((ch=*p) && (ch!='\n' && ch!='\r')){
			putchar(ch);
			p++;
		}
		if(ch=='\n' || ch=='\r'){
			putchar(0x00);
			p++;
		}
		while((ch=*p)=='\n'){		// 連続する改行を読み飛ばす
			p++;
		}
	}
	putchar(0x80);					// NTBのプログラム末は$80
	exit(0);
}
