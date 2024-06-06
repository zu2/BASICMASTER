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
		fprintf(stderr,"usage: %s filename savename\n",argv[0]);
		fprintf(stderr,"savename examble: BASIC.S\n");
		exit(1);
	}
	char	*filename = argv[1];
	char	*savename = argv[2];

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
	char	*buffer;
	if((buffer=(char *)malloc(filesize+1))==NULL){
		perror("can't malloc");
		exit(1);
	}
	read(fd,buffer,filesize);
	buffer[filesize]=0;

	char	*p = buffer;
	long	s = filesize;
	char	*q;
	int		len = strlen(savename);
	int		n1 = 9;
	int		n2 = 8;
	while(s>0 && p<(buffer+filesize) && (q=memchr(p,savename[0],s))!=NULL){
		//fprintf(stderr,"found: %04lx\n",q-p);
		if(memcmp(q,savename,len)==0){
			fprintf(stderr,"found: %s %02d %04lx\n",savename,*(q+9),q-p);
			for(char *qq = q-n1-2; qq<q+n2+3; qq++){
				fprintf(stderr," %02x",*qq);
			}
			fprintf(stderr,"\n");
			n1 = 9;
			if(q[-10] == '\r')	n1=9;
			if(q[-11] == '\r')	n1=10;
			char *c1 = q-n1;
			char *c2 = q+n2;
			memmove(c1,c2,s-(q-p)+n2);
			s -= n1+n2+1;
			p = q-n1;
			fprintf(stderr,"size: %ld\n",s);
		}else{
			p++;
		}
	}
	for(p=buffer; p<buffer+s; p++){
		if(*p=='\r'){
			*p = '\n';
		}
		putchar(*p);
	}
	puts("\n");
	exit(0);
}
