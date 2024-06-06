//
//	Convert BASICMASTER text to UNICODE text
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
	buffer[filesize]=0;

	setlocale(LC_ALL,"ja_JP");
	unsigned char	*p = buffer;
	unsigned char	*top,*end;
	top = buffer;
	end = buffer+filesize;
//	printf("file size: %ld\n",filesize);
	while(p<end){
		if(*p=='\r' || *p=='\n'){
			putchar('\n');
			p++;
			top = p;
		}else if(*p<0x20){
			p++;
			;
		}else{
			int i=0;
			for(i=0; table[i].to!=NULL; i++){
				if(strlen(table[i].to)>1){
					continue;
				}
				if(table[i].to[0] == *p){
					fputs(table[i].from,stdout);
						p++;	
						break;
				}
			}
			if(table[i].to==NULL){
				if(*p<0x80){
					putchar(*p++);
				}else{
					printf("\\x%02x",*p++);
				}
			}
		}
	}
//	fprintf(stderr,"all end\n");
	exit(0);
}
