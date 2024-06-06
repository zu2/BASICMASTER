//
//	Convert any text to Motorola S format
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

extern	int
main(int argc, char **argv)
{
	if(argc<3) {
		fprintf(stderr,"usage: %s txtfile offset\n",argv[0]);
		exit(1);
	}
	char	*filename = argv[1];
	char	*e;
	unsigned int	offset;
	if(memcmp(argv[2],"0x",2)==0){
		offset = strtol(argv[2]+2,&e,16);
	}else if (memcmp(argv[2],"$", 1)==0) {
		offset = strtol(argv[2]+1,&e,16);
	}else{
		offset = strtol(argv[2],&e,16);
	}
	if(*e != '\0'){
		fprintf(stderr,"can't strtol(%s)\n",argv[3]);
		perror("");
		exit(1);
	}
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

	unsigned char	*p = buffer;
	long			s = filesize;
	while(s>=16) {
		printf("S113%04x",offset);
//		unsigned int sum=0;
		unsigned int sum=0x13+((offset&0x0ff00)>>8)+(offset&0x0ff);
		for(int i=0; i<16; i++) {
			printf("%02x",p[i]);
			sum += p[i];
		}
		offset+=16;
		p+=16;
		s-=16;
		printf("%02x\n",0x0ff-(sum&0x0ff));
	}
	if(s>0){
		printf("S1%02x%04x",(int)(s+3),offset);
		unsigned int sum=(s+3)+((offset&0x0ff00)>>8)+(offset&0x0ff);
		for(int i=0; i<s; i++) {
			printf("%02x",p[i]);
			sum += p[i];
		}
		printf("%02x\n",0x0ff-(sum&0x0ff));
	}
	printf("S9030000FC\n");
	exit(0);
}
