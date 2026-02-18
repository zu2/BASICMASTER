#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<string.h>
#include	<unistd.h>
#include	<stdint.h>

//
// GAME言語のソースをベタセーブしたテープからテキストを抜き出す
//
// bmtape2 -300 source.wav (source.Sが出てくる）
// ./game2txt source.S | ./bm2txt
//

int getlineno(uint8_t *p)
{
	return p[0]*256 + p[1];
}

extern	int
main(int argc, char **argv)
{
	if(argc!=2) {
		fprintf(stderr,"usage: %s filename\n",argv[0]);
		exit(1);
	}
	char	*filename = argv[1];

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
	uint8_t	*buffer;
	if((buffer=(uint8_t *)malloc(filesize+1))==NULL){
		perror("can't malloc");
		exit(1);
	}
	read(fd,buffer,filesize);
	buffer[filesize]=0;
	if (filesize<=0) {
		perror("file read error");
		exit(1);
	}

	uint8_t	*p = buffer;
	uint8_t	*end = buffer+filesize;
	int		lineno;
	while((lineno=getlineno(p)) && p<end) {
		p+=2;
		printf("%d",lineno);
		uint8_t *q = p;
		while (*p && p<end) {
			p++;
		}
		if (!*p) {
			printf("%s\n",q);
		}
		p++;
	}
	exit(0);
}
