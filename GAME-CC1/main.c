#include	"common.h"

char	*source_file_name;

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "引数の個数が正しくありません\n");
		return 1;
	}

	
//user_input = argv[1];
	source_file_name = argv[1];
	int	fd = open(source_file_name,O_RDONLY);
	if(fd<0){
		perror(NULL);
		fprintf(stderr,"can't open %s\n",source_file_name);
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
	Node	*current = NULL;
	for(int i=0; code[i]; i++){
		if(code[i]->kind == ND_LINENUM){
			if(prev!=NULL){
				prev->rhs = code[i];
			}
			code[i]->lhs = prev;
			prev = code[i];
		}
	}
	for(int i=0; code[i]; i++){
		if(code[i]->kind == ND_LINENUM){
			current = code[i];
		}
		if(code[i]->kind==ND_IF){
			existLINENO(current->rhs->val);
		}
		// ND_IFGOTO の時は次の行番号はマークしない
	}
	prev->rhs = new_node(ND_LINENUM);
	prev->rhs->val = 0;
	for(int i=0; code[i]; i++){
//		printf("; gen code[%d] start:\n",i);
//		printf(";   ");print_nodes(code[i]);printf("\n");
		code[i]=node_opt(code[i]);
//		printf(";=> ");print_nodes(opt);printf("\n");
	}
	for(int i=0; code[i]; i++){
//		printf(";gen code[%d] end\n",i);
		gen_stmt(node_opt(code[i]));
	}
	epilogue();
	return 0;
}
