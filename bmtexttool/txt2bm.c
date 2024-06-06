//
//	Convert UNICODE text to BASICMASTER text
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
#include	<wchar.h>
#include	<locale.h>

#include	"bmchar.h"

int	hex2bin(unsigned char ch)
{
	if(isdigit(ch)){
		return ch-'0';
	}else if(ch>='a'){
		return ch-'a'+10;
	}else if(ch>='A'){
		return ch-'A'+10;
	}
	perror("illegal hex char");
	exit(1);
}
extern	int
main(int argc, char **argv)
{
	if(argc<2) {
		fprintf(stderr,"usage: %s txtfile\n",argv[0]);
		exit(1);
	}
	char	*filename = argv[1];
//	fprintf(stderr,"offset:%04x\n",offset);
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
	// delete '\n\n' at the end.
	while(filesize>2 && buffer[filesize-1]=='\n' && buffer[filesize-2]=='\n'){
		filesize--;
	}
	buffer[filesize]=0;

	setlocale(LC_ALL,"ja_JP");
	unsigned char	*p = buffer;
	unsigned char	*top;
	top = buffer;
	while(*p){
		if(*p=='\n'){
			if(p!=buffer && p[-1]!='\n') putchar('\r');
			p++;
			top = p;
		}else if(*p<0x20){
			p++;
			;
		}else if((*p=='\\') && (p[1]=='x') && isxdigit(p[2]) && isxdigit(p[3])){	// \xFF を変換
			putchar(hex2bin(p[2])*16+hex2bin(p[3]));
			p+=4;
		}else if((*p)<0x80){
			putchar(*p++);
		}else{
			int len=mblen((char *)p,MB_CUR_MAX);
			if(len<1){
				fprintf(stderr,"illegal multi byte string\n");
				fputs((char *)top,stderr);
				perror("\ntxt2mbtxt:");
				exit(1);
			}
			int i;
			for(i=0; table[i].from; i++){
				if(mblen(table[i].from,MB_CUR_MAX) == len){
					if(memcmp(p,table[i].from,len)==0){
						fputs(table[i].to,stdout);
						p+=len;
						break;
					}
				}
			}
			if(table[i].from==NULL){
				unsigned char	s[len+1];
				memcpy(s,p,len);
				s[len]='\0';
				fprintf(stderr,"char table unmatch '%s'\n",s);
				fputs((char *)top,stderr);
				perror("\ntxt2mbtxt:");
				exit(1);
			}
		}
	}
	putchar(0);
//	fprintf(stderr,"all end\n");
	exit(0);
}
