#include	"common.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	
//user_input = argv[1];
	char	*filename = argv[1];
	int	fd = open(filename,O_RDONLY);
	if(fd<0){
		perror(NULL);
		fprintf(stderr,"can't open %s\n",filename);
		exit(1);
	}
	struct  stat    stbuf;
	if(fstat(fd,&stbuf)<0) {
		perror("can't stat");
		exit(1);
	}
	long filesize = stbuf.st_size;
	if((user_input=calloc(1,filesize+2))==NULL){
		perror("can't calloc");
		exit(1);
	}
	read(fd,user_input,filesize);
	user_input[filesize]=0;

//	printf(";--------------\n");
	token	= tokenize();
//	printf(";--------------\n");
	program();
//	printf(";--------------\n");fflush(stdout);
//	print_program();

	prologue();
	

	Node	*prev = NULL;
	Node	*last = NULL;
	for(int i=0; code[i]; i++){
		if(code[i]->kind == ND_LINENUM){
			if(prev!=NULL){
				prev->rhs = code[i];
			}
			code[i]->lhs = prev;
			prev = code[i];
		}
	}
	prev->rhs = new_node(ND_LINENUM);
	prev->rhs->val = 0;
	for(int i=0; code[i]; i++){
//		printf("; gen code[%d] start:\n",i);
//		printf(";   ");print_nodes(code[i]);printf("\n");
		Node *opt=node_opt(code[i]);
//		printf(";=> ");print_nodes(opt);printf("\n");
		gen_stmt(opt);
//		printf(";gen code[%d] end\n",i);
	}

	epilogue();
	return 0;
}
