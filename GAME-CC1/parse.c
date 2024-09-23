#include	"common.h"

typedef	struct	{
			TokenKind	tk;
			char		*str;
} Keyword_t;

Keyword_t	Statement[] = {
	{	TK_IF,			";=",		},
	{	TK_GOTO,		"#=",		},
	{	TK_GOSUB,		"!=",		},
	{	TK_RETURN,		"]",		},
	{	TK_SETRAM,		"*=",		},
	{	TK_PRINT,		"?=",		},
	{	TK_PRINTR,		"?(",		},
	{	TK_PRINTHEX4,	"?""?=",	},
	{	TK_PRINTHEX2,	"?$=",		},
	{	TK_PRINTTAB,	".=",		},
	{	TK_PRINTCH,		"$=",		},
//	{	TK_PRINTCR,		"/",		},	// TK_DIVと干渉する
//	{	TK_CALL,		">=",		},
	{	TK_UNTIL,		"@=(",		},
	{	TK_NEXT,		"@=",		},
	{	TK_DO,			"@",		},
	{	TK_SETRAND,		"'=",		},
	{	TK_SETTIMER,	"\\=",		},
	{	TK_SETCURSOR,	"^=",		},
	{	TK_ASM,			"#",		},
	{	TK_NONE,		"",			}
};
Keyword_t	Ident[] = {
	{	TK_RESERVED,	"==",		},
	{	TK_RESERVED,	">=",		},
	{	TK_RESERVED,	"<=",		},
	{	TK_RESERVED,	"=>",		},
	{	TK_RESERVED,	"=<",		},
	{	TK_RESERVED,	"<>",		},
	{	TK_RESERVED,	"(",		},
	{	TK_RESERVED,	")",		},
	{	TK_RESERVED,	":",		},
	{	TK_RESERVED,	",",		},
	{	TK_RESERVED,	"'",		},
	{	TK_RESERVED,	"%",		},
	{	TK_RESERVED,	"&",		},
	{	TK_RESERVED,	"?",		},
	{	TK_RESERVED,	"^$",		},
	{	TK_RESERVED,	"^",		},
	{	TK_RESERVED,	"\\",		},	// TIMER
	{	TK_RESERVED,	"&",		},	// AND
	{	TK_RESERVED,	".",		},	// OR
	{	TK_RESERVED,	"!",		},	// XOR
//	{	TK_RESERVED,	"$",		},	// HEX,INKEYが干渉する
	{	TK_NONE,		"",			}
};

char *makestr(char *p,int len)
{
	char	*buf = calloc(1,len+1);
	memcpy(buf,p,len);
	buf[len] = '\0';
	return	buf;
}

char *token2str(Token *token)
{
	if(token==NULL){
		return "NULL";
	}
	return	makestr(token->str,token->len);
}

void
print_token(Token *token)
{
	switch(token->kind){
	case TK_NONE:		printf("TK_NONE\n");break;
	case TK_EOF:		printf("TK_EOF\n");break;
	case TK_EOL:		printf("TK_EOL\n");break;
	case TK_LINENUM:	printf("TK_LINENUM: %d %s\n",token->val,token->str);break;
	case TK_SEP:		printf("TK_SEP\n");break;
	case TK_RESERVED:	printf("TK_RESERVED %s\n",token->str);break;
	case TK_LVAR:		printf("TK_LVAR %s\n",token->str);break;
	case TK_NUM:		printf("TK_NUM %d\n",token->val);break;
	case TK_IF:			printf("TK_IF\n");break;
	case TK_GOTO:		printf("TK_GOTO %d\n",token->val);break;
	case TK_GOSUB:		printf("TK_GOSUB %d\n",token->val);break;
	case TK_RETURN:		printf("TK_RETURN\n");break;
	case TK_SETRAM:		printf("TK_SETRAM\n");break;
	case TK_PRINT:		printf("TK_PRINT\n");break;
	case TK_PRINTR:		printf("TK_PRINTR\n");break;
	case TK_PRINTHEX4:	printf("TK_PRINTHEX4\n");break;
	case TK_PRINTHEX2:	printf("TK_PRINTHEX2\n");break;
	case TK_PRINTTAB:	printf("TK_PRINTTAB\n");break;
	case TK_PRINTCR:	printf("TK_PRINTCR\n");break;
	case TK_PRINTCH:	printf("TK_PRINTCH\n");break;
	case TK_STR:		printf("TK_STR len=%d \"%s\"\n",token->len,token->str);break;
	case TK_CALL:		printf("TK_CALL\n");break;
	case TK_UNTIL:		printf("TK_UNTIL\n");break;
	case TK_DO:			printf("TK_DO\n");break;
	case TK_NEXT:		printf("TK_NEXT\n");break;
	case TK_FOR:		printf("TK_FOR\n");break;
	case TK_SETRAND:	printf("TK_SETRAND\n");break;
	case TK_INKEY:		printf("TK_INKEY\n");break;
	case TK_MOD:		printf("TK_MOD\n");break;
	case TK_REM:		printf("TK_REM '%s'\n",token->str);break;
	case TK_MUSIC:		printf("TK_MUSIC '%s'\n",token->str);break;
	case TK_TIMER:		printf("TK_TIMER\n");break;
	case TK_SETTIMER:	printf("TK_SETTIMER\n");break;
	case TK_CURSOR:		printf("TK_CURSOR\n");break;
	case TK_CURSORADRS:	printf("TK_CURSORADDR\n");break;
	case TK_KEYBOARD:	printf("TK_KEYBOARD\n");break;
	case TK_ASM:		printf("TK_ASM '%s'\n",token->str);break;
	default:
			printf(";unknown token kind %d\n",token->kind);
			break;
	}
}


//
//	comparison symbol
//
char	*
compare_symbol_str(NodeKind kind)
{
	switch(kind){
	case ND_EQ: return "==";
	case ND_NE:	return "!=";
	case ND_LT:	return "<";
	case ND_LE:	return "<=";
	case ND_GT:	return ">";	
	case ND_GE:	return ">=";
	default:
		error("; what? unknown kind %d\n",kind);
	}
	return	"";
}

void	print_nodes(Node *node);

void
print_unary_node(char *name,Node *node)
{
	printf("(%s ",name);
	print_nodes(node->lhs);
	printf(")");
}
void
print_binary_node(char *name,Node *node)
{
	printf("(%s ",name);
	print_nodes(node->lhs);
	printf(" ");
	print_nodes(node->rhs);
	printf(")");
}
void
print_var_node(char *name,Node *node){
	printf("(%s %s)",name,node->str);
}
void
print_array_node(char *name,Node *node){
	printf("(%s %s ",name,node->str);
	print_nodes(node->lhs);
	printf(")");
}
void
print_setvar_node(char *name,Node *node){
	printf("(%s %s ",name,node->str);
	print_nodes(node->lhs);
	printf(")");
}
void
print_setvar_n_node(char *name,Node *node){
	printf("(%s ",name);
	print_nodes(node->lhs);
	printf(" ");
	print_nodes(node->rhs);
	printf(")");
}
void
print_setary_n_node(char *name,Node *node){
	printf("(%s ",name);
	print_nodes(node->lhs);
	printf(" ");
	print_nodes(node->rhs);
	printf(")");
}
void
print_unary_val_node(char *name,Node *node)
{
	printf("(%s ",name);
	print_nodes(node->lhs);
	printf(" val=%d)",node->val);
}
void
print_cell_node(Node *node)
{
	Node *rhs = node->rhs;
	printf("(");
	print_nodes(node->lhs);
	printf(" ");
	while(rhs->kind==ND_CELL){
		print_nodes(rhs->lhs);
		rhs = rhs->rhs;
	}
	if(rhs->kind!=ND_NONE){
		printf(" . ");			// dot pair
		print_nodes(rhs);
	}
	printf(")");
}
//
//	ON GOTO/GOSUB
//		lhs = term
//		rhs = cellist (line number)
//
void
print_on_node(char *name,Node *node)
{
}

void
print_nodes(Node *node)
{
	if(node==NULL){
		error("print_nodes: Node is NULL");
	}
	Node	*lhs = node->lhs;
	Node	*rhs = node->rhs;
	int		val = node->val;
	char	*str = node->str;
	switch(node->kind){
	case ND_NONE:		printf("ND_NONE");break;
	case ND_CELL:		print_cell_node(node);break;
	case ND_NOP:		printf("(ND_NOP)");break;
	case ND_ADD:		print_binary_node("+",node);break;
	case ND_SUB:		print_binary_node("-",node);break;
	case ND_MUL:		print_binary_node("*",node);break;		
	case ND_DIV:		print_binary_node("/",node);break;		
	case ND_ABS:		print_unary_node("ND_ABS",node);break;
	case ND_NEG:		print_unary_node("ND_NEG",node);break;
	case ND_EQ:			print_binary_node("==",node);break;			
	case ND_NE:			print_binary_node("!=",node);break;	
	case ND_LT:			print_binary_node("<",node);break;	
	case ND_LE:			print_binary_node("<=",node);break;	
	case ND_GT:			print_binary_node(">",node);break;	
	case ND_GE:			print_binary_node(">=",node);break;	
	case ND_VAR:		print_var_node("ND_VAR",node);break;
	case ND_ARRAY1:		print_array_node("ND_ARRAY1",node);break;
	case ND_ARRAY2:		print_array_node("ND_ARRAY2",node);break;
	case ND_NUM:		printf("(ND_NUM %d)",val);break;
	case ND_ASSIGN:		print_binary_node("ND_ASSIGN",node);break;
	case ND_IF:			print_unary_node("ND_IF",node);break;
	case ND_GOTO:		printf("(ND_GOTO %d)",val);break;
	case ND_GOSUB:		printf("(ND_GOSUB %d)",val);break;
	case ND_RETURN:		printf("ND_RETURN ");break;
	case ND_ONGOTO:		print_on_node("ND_ONGOTO",node);break;
	case ND_ONGOSUB:	print_on_node("ND_ONGOSUB",node);break;
	case ND_LINENUM:	printf("(ND_LINENUM %d '%s')",val,str);break;
	case ND_SETRAM:		print_unary_node("ND_SETRAM",node);break;
	case ND_PRINT:		print_unary_node("ND_PRINT",node);break;
	case ND_PRINTR:		print_binary_node("ND_PRINTR",node);break;
	case ND_PRINTHEX4:	print_unary_node("ND_PRINTHEX4",node);break;
	case ND_PRINTHEX2:	print_unary_node("ND_PRINTHEX2",node);break;
	case ND_PRINTTAB:	print_unary_node("ND_PRINTTAB",node);break;
	case ND_PRINTCR:	printf("(ND_PRINTCR)");break;
	case ND_PRINTCH:	print_unary_node("ND_PRINTCH ",node);break;
	case ND_PRINTSTR:	printf("(ND_PRINTSTR '%s')",str);break;
	case ND_CALL:		printf("(ND_CALL)");break;
	case ND_CALLVAL:	printf("(ND_CALLVAL)");break;	// 機械語から戻ったときのAccAB
	case ND_UNTIL:		printf("(ND_UNTIL label=%d ",val);
						print_nodes(lhs);printf(")");
						break;
	case ND_DO:			printf("(ND_DO label=%d)",val);break;
	case ND_NEXT:		printf("(ND_NEXT label=%d str=%s ",val,str);
						print_nodes(lhs);
						printf(")");
						break;
	case ND_FOR:		printf("(ND_FOR str=%s label=%d ",str,val);
						print_nodes(lhs);
						printf(" ");
						print_nodes(rhs);
						printf(")");
						break;
	case ND_SETRAND:	printf("(ND_SETRAND ");print_nodes(lhs);break;
	case ND_REM:		printf("(ND_REM str=\"%s\")",str);break;
	case ND_PGEND:		printf("(ND_PGEND ");break;
	case ND_RAND:		print_unary_node("ND_RAND",node);break;
	case ND_MOD:		print_unary_node("ND_MOD",node);break;
	case ND_INKEY:		printf("(ND_INKEY)");break;
	case ND_TIMER:		printf("(ND_TIMER)");break;
	case ND_SETTIMER:	print_unary_node("ND_SETTIMER",node);break;
	case ND_INPUT:		printf("(ND_INPUT)");break;
	case ND_CURSOR:		printf("(ND_CURSOR)");break;
	case ND_SETCURSOR:	printf("(ND_SETCURSOR ");print_nodes(lhs);printf(")");break;
	case ND_CURSORADRS:	printf("(ND_CURSORADRS)");break;
	case ND_SETCURSORADRS:printf("(ND_SETCURSORADRS ");print_nodes(lhs);printf(")");break;
	case ND_MUSIC:		printf("(ND_MUSIC str='%s' )",str);break;
	case ND_KEYBOARD:	printf("(ND_KEYBOARD)");break;
	case ND_BITAND:		print_binary_node("ND_BITAND",node);break;
	case ND_BITOR:		print_binary_node("ND_BITOR",node);break;
	case ND_BITXOR:		print_binary_node("ND_BITXOR",node);break;
	// 以下、オプティマイズ用
	case ND_SETVAR:		print_setvar_node("ND_SETVAR",node);break;
	case ND_SETVAR_N:	print_setvar_n_node("ND_SETVAR_N",node);break;
	case ND_SETARY1_N:	print_setary_n_node("ND_SETARY1_N",node);break;
	case ND_SETARY2_N:	print_setary_n_node("ND_SETARY2_N",node);break;
	case ND_ASLD:		print_unary_val_node("ND_ASLD",node);break;
	case ND_ANDI:		print_unary_val_node("ND_ANDI",node);break;
	case ND_ANDI_MOD:	print_unary_val_node("ND_ANDI_MOD",node);break;
	case ND_NEGVAR:		print_var_node("ND_NEGVAR",node);break;
	case ND_INCVAR:		print_var_node("ND_INCVAR",node);break;
	case ND_DECVAR:		print_var_node("ND_DECVAR",node);break;
	case ND_INC2VAR:	print_var_node("ND_INC2VAR",node);break;
	case ND_DEC2VAR:	print_var_node("ND_DEC2VAR",node);break;
	case ND_ASM:		printf("(ND_ASM str=\"%s\")",str);break;
	case ND_IFGOTO:		printf("(ND_IFGOTO val=%d ",val);print_nodes(lhs);printf(")");break;
//	case ND_STACKTOP:	printf("(ND_STACKTOP val=%d ",val);print_nodes(lhs);printf(")");break;
	case ND_RELMUL:		print_binary_node("ND_RELMUL",node);break;	//	関係演算同士の*
	case ND_RELADD:		print_binary_node("ND_RELADD",node);break;	//	関係演算同士の+
	case ND_UPDATEDO:	printf("(ND_UPDATEDO val=%d)",val);break;	// DO-UNTILの制御変数の更新
	default:
			printf(";unknown node kind %d\n",node->kind);
			break;
	}
}

void
print_nodes_ln(Node *node)
{
	print_nodes(node);
	printf("\n");
}

// 入力プログラム
char *user_input;
char *current_linetop;
int line_count=0;
typedef	struct {
	int		n;
	char	*s;
	int		used;
} line_t;
line_t LINENO[10000];

// 追加はtokenize時点で行われる
void add_LINENO(int n,char *s){
	LINENO[line_count].n = n;
	LINENO[line_count].s = s;
	LINENO[line_count].used = 0;
	line_count++;
}

char *existLINENO(int v)
{
//	printf("; search linenumber %d\n",v);
	for(int i=0; i<line_count; i++){
//		printf("; line %d\n",LINENO[i].n);
		if(LINENO[i].n==v){
			LINENO[i].used=1;
			return LINENO[i].s;
		}
	}
	if(v==1) return "OK";
	if(v==65535) return "OK";
	return NULL;
}
// LINE number used?
int usedLINENO(int v)
{
//	printf("; search linenumber %d\n",v);
	for(int i=0; i<line_count; i++){
//		printf("; line %d\n",LINENO[i].n);
		if(LINENO[i].n==v){
			return LINENO[i].used;
		}
	}
	if(v==1) return 1;
	if(v==65535) return 1;
	return 0;
}

char	*
get_least_line(char	*p)
{
	char	*buf = calloc(1,1024);
	char	*q = buf;

	while(*p!='\n' && *p!='\r' && *p!='\0'){
		*q++ = *p++;
	}
	*q = '\0';
	return	buf;
}


void	error(char *fmt, ...)
{
	va_list		ap;
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	fprintf(stderr,"\n");
	fflush(stderr);
	fprintf(stderr,"; %s\n",current_linetop);
	exit(1);
}


// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char	*p = loc;

	while(p>user_input && *p!='\r' && *p!='\n'){
		p--;
	}
	if(*p=='\r'||*p=='\n'){
		p++;
	}
	int pos = loc - p;
	fprintf(stderr, "%s\n", get_least_line(p));
	fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	fprintf(stderr,"; %s\n",get_least_line(current_linetop));
	exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(TokenKind tk) {
//	printf(";consume: kind='%d' -> ",tk);
	if (token->kind != tk){
//		printf("false\n");
		return false;
	}
//	printf("true\n");
	token = token->next;
	return true;
}
// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume_op(char *op) {
//	printf(";consume_op:'%s' -> ",op);
	if (token->kind != TK_RESERVED ||
		(int)strlen(op) != token->len ||
		memcmp(token->str, op, token->len)){
//		printf("false\n");
		return false;
	}
//	printf("true\n");
	token = token->next;
	return true;
}
// 次のトークンがEOFの場合、そのtokenを返す。tokenは読み進めない。
// それ以外の場合にはNULLを返す
Token *consume_eof() {
	if (token->kind != TK_EOF){
		return	NULL;
	}
	return token;
}
// 次のトークンが区切りの場合、トークンを1つ読み進めてそのtokenを返す。
// それ以外の場合にはNULLを返す
Token *consume_sep() {
//	printf(";consume_sep -> ");
	if (token->kind != TK_SEP){
//		printf("false\n");
		return	NULL;
	}
	Token	*ret=token;
	while(token->kind==TK_SEP){
		ret=token;
		token = token->next;
	}
//	printf("true: token->str='%s'\n",token2str(token));
	return ret;
}
// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはNULLを返す
Token *consume_number() {
//	printf(";consume_number -> ");
	if (token->kind != TK_NUM){
//		printf("false\n");
		return	NULL;
	}
	Token	*ret=token;
//	printf("true: token->val='%d'\n",token->val);
	token = token->next;
	return ret;
}
// 次のトークンが変数の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはNULLを返す
Token *consume_var() {
//	printf(";consume_var -> ");
	if (token->kind != TK_LVAR){
//		printf("false\n");
		return	NULL;
	}
	Token	*ret=token;
//	printf("true: token->str='%s'\n",token2str(token));
	token = token->next;
	return ret;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
	if (token->kind != TK_RESERVED
	|| (int)strlen(op) != token->len
	|| memcmp(token->str,op,token->len)){
		printf("; ");print_token(token);printf("\n");
		error_at(token->str,"expected '%c'", op);
	}
	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM){
		error_at(token->str,"expected a number");
	}
	int val = token->val;
	token = token->next;
	return val;
}
// 次のトークンが変数の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
char *expect_var() {
	if (token->kind != TK_LVAR){
		error_at(token->str,"expected a variable");
	}
	char *str = token->str;
	token = token->next;
	return str;
}


bool at_eof() {
	return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str,int len) {
	Token *tok = calloc(1, sizeof(Token));
	char  *s   = calloc(1, len+1);
	strncpy(s,str,len);
	tok->kind = kind;
	tok->str = s;
	tok->len = len;
	cur->next = tok;
	return tok;
}

bool startswitch(char *p,char *q)
{
	return	memcmp(p,q,strlen(q))==0;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize()
{
	int line_count=0;
	char	*p = user_input;
	current_linetop = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;
	int		linetop=1;
	static int old_linenumber=0;

//	printf(";start tokenize() *p='%c'\n",*p);
	while (*p) {
		if(*p=='\t'){
			error_at(p,"TAB found %d\n");
		}
		if(linetop){
//			printf(";start linetop '%s'\n",get_least_line(p));
			current_linetop = p;
			while(*p==' '){
				p++;
			}
			// 行番号のはず
			if (!isdigit(*p)) {
				error_at(p,"not linenumber '%c'\n",*p);
			}else{
				char *str=get_least_line(p);
				cur = new_token(TK_LINENUM, cur, str,strlen(str));
				char *q=p;
				cur->val = strtol(p, &p, 10);
//				printf(";tokenize TK_LINENUM '%d',p='%s'\n",cur->val,cur->str);
				cur->len = p-q;
				if(old_linenumber>=cur->val){
					error_at(p,"linenumber error %d,%d\n",cur->val,old_linenumber);
				}
				old_linenumber = cur->val;
				add_LINENO(cur->val,str);
			}
			if(*p!=' '){
//				printf(";REM mark '%c' found. skip line\n",*p);
				if(*p=='\t'){
					error_at(p,"TAB found.\n");
				}
				char	*rem = get_least_line(p);
				if(*p=='#')	{ // Assembler extension
					cur = new_token(TK_ASM, cur, rem+1,strlen(rem+1));
				}else{
					cur = new_token(TK_REM, cur, rem,strlen(rem));
				}
//				printf(";tokenize TK_REM p='%s'\n",rem);
				while(*p!='\n' && *p!='\r') { p++; }	// 行末まで読み飛ばす
//				printf("; REM %s\n",rem);
				linetop=0;
				continue;
			}
			while(*p==' '){
				p++;
			}
			linetop=0;
		}
		// 行末
		if (*p=='\n'){
//			printf(";tokenize TK_EOL\n");
			cur = new_token(TK_EOL,cur,p++,1);
			linetop=1;
			continue;
		}else if (*p=='\r'){
//			printf(";tokenize TK_EOL\n");
			if(p[1]=='\n'){
				cur = new_token(TK_EOL,cur,"\n",1);
				p+=2;
				linetop=1;
				continue;
			}else{
				cur = new_token(TK_EOL,cur,"\n",1);
				p++;
				linetop=1;
				continue;
			}
		}
		if(*p==' '){
			cur = new_token(TK_SEP,cur,p++,1);
			while(*p==' '){
				p++;
			}
			continue;
		}
		if(*p=='/' && (p[1]==' '||p[1]=='"' || p[1]=='/'||p[1]=='\n')){
//			printf(";tokenize TK_PRINTCR '/',p='%s'\n",get_least_line(p));
			if((p[-1]=='"' || p[-1]=='/')){
				cur = new_token(TK_SEP,cur,p,1);
			}
			cur = new_token(TK_PRINTCR,cur,p++,1);
			continue;
		}
		for(Keyword_t	*st=Statement; st->tk!=TK_NONE; st++){
//			printf(";Statement '%s' ?\n",st->str);
			if(startswitch(p,st->str)){
				if(st->tk==TK_UNTIL)	p--;
//				printf(";tokenize TK_%d '%s',p='%s'\n",st->tk,st->str,get_least_line(p));
				cur = new_token(st->tk,cur,p,strlen(st->str));
//				printf(";tokenize done ");print_token(cur);
				p+=strlen(st->str);
				goto	next_token;
			}
		}
		for(Keyword_t	*st=Ident; st->tk!=TK_NONE; st++){
//				printf(";Ident '%s' ?\n",st->str);
			if(startswitch(p,st->str)){
//				printf(";tokenize TK_%d '%s',p='%s'\n",st->tk,st->str,get_least_line(p));
				cur = new_token(st->tk,cur,p,strlen(st->str));
//				printf(";tokenize done ");//print_token(cur);
				p+=strlen(st->str);
				goto	next_token;
				}
		}
		if (*p=='"') {		// 文字列
//			printf(";tokenize TK_STR '%c',p='%s'\n",*p,get_least_line(p));
			if((p[-1]=='/')||(p[-1]=='"')){
				cur = new_token(TK_SEP,cur,p,1);
			}
			p++;
			char *q = p;
			while(*p!='"' && *p!='\r' && *p!='\n' && *p!='\0'){
				p++;
			}
			if(*p=='"' && p-q>0){
				cur = new_token(TK_STR,cur,makestr(q,p-q),p-q);
//				printf(";tokenize OK TK_STR len=%d, '%s'\n",cur->len,cur->str);
				p++;
				continue;
			}else if(*p=='"' && p==q){	// 空文字列はインタプリタではエラーにならないので合わせる
				cur = new_token(TK_SEP,cur,p,1);	// 何も生成しないと怒られるので…
				p++;
				continue;
			}
			// "の後に行末やファイル末が来た
//			printf(";tokenize NG TK_STR len=%ld,'%c'(%02x)\n",p-q,*p,*p);
			error_at(p-1,"string \"...\" not terminated. '%s'\n",current_linetop);
			continue;
		}else if (*p=='+' || *p=='-' || *p=='*' || *p=='/') {
//			printf(";tokenize TK_RESERVED '%c',p='%s'\n",*p,get_least_line(p));
			cur = new_token(TK_RESERVED, cur, p++,1);
			continue;
		}else if (*p=='(' || *p==')'){
//			printf(";tokenize TK_RESERVED '%c',p='%s'\n",*p,get_least_line(p));
			cur = new_token(TK_RESERVED, cur, p++,1);
			continue;
		}else if (*p=='<'){
			if (*(p+1)=='=' || *(p+1)=='>'){
//				printf(";tokenize TK_RESERVED '%c%c',p='%s'\n",*p,*p+1,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p,2);
				p+=2;
				continue;
			}else{
//				printf(";tokenize TK_RESERVED '%c',p='%s'\n",*p,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p++,1);
				continue;
			}
		}else if (*p=='>'){
			if (*(p+1)=='=' || *(p+1)=='<'){
//				printf(";tokenize TK_RESERVED '%c%c',p='%s'\n",*p,*p+1,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p,2);
				p+=2;
				continue;
			}else{
//				printf(";tokenize TK_RESERVED '%c',p='%s'\n",*p,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p++,1);
				continue;
			}
		}else if (*p=='='){
			if (*(p+1)=='>' || *(p+1)=='<'){
//				printf(";tokenize TK_RESERVED '%c%c',p='%s'\n",*p,*p+1,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p,2);
				p+=2;
				continue;
			}else{
//				printf(";tokenize TK_RESERVED '%c',p='%s'\n",*p,get_least_line(p));
				cur = new_token(TK_RESERVED, cur, p++,1);
				continue;
			}
		}else if (*p==']'){
//			printf(";tokenize TK_RETURN '%c',p='%s'\n",*p,get_least_line(p));
			cur = new_token(TK_RETURN, cur, p++,1);
			continue;
		}else if (*p=='$') { // hex digit or inkey$
			if(isxdigit(*(p+1))){		// hex
				cur = new_token(TK_NUM, cur, p,0);
				char *q=p;
				cur->val = strtol(p+1, &p, 16);
//				printf(";tokenize TK_NUM hex '%d',p='%s'\n",cur->val,get_least_line(p));
				cur->len = p-q+1;
				continue;
			}else{
//				printf(";tokenize TK_INKEY '%c',p='%s'\n",*p,get_least_line(p));
				cur = new_token(TK_INKEY, cur, p++,1);
				continue;
			}
		}else if (isdigit(*p)){
			cur = new_token(TK_NUM, cur, p,0);
			char *q=p;
			cur->val = strtol(p, &p, 10);
//			printf(";tokenize TK_NUM '%d',p='%s'\n",cur->val,get_least_line(p));
			cur->len = p-q;
			continue;
		}else if (('A'<=*p) && (*p<='Z')){
//			printf(";tokenize TK_LVAR '%c',p='%s'\n",*p,get_least_line(p));
			cur = new_token(TK_LVAR,cur,p,1);
			cur->str=makestr(p,1);
			cur->len=1;
			while(('A'<=*p) && (*p<='Z')){
				p++;
			}
			continue;
		}else if(startswitch(p,"[=0")){
			error_at(p,"Monitor command not implemented\n");
		}else if(startswitch(p,"[=1")){
//			printf("; MUSIC extension found.\n");
			cur = new_token(TK_MUSIC,cur,p,3);
			p+=3;
			while(*p==' '){
				p++;
			}
			char *q=p;
			while(*p!='\0' && *p!='\r' && *p!='\n' && *p!='%'){
				p++;
			}
			if(*p=='%'){
				p++;
			}
			if(p==q){
				error_at(current_linetop,"; empty MUSIC string\n");
			}
			cur->str=makestr(q,p-q);
			cur->len=p-q;
			if(*p=='\0'){
				p--;
			}
//			printf("; MUSIC string: '%s'\n",cur->str);
			continue;
		}else if(*p=='['){
			cur = new_token(TK_KEYBOARD,cur,p,1);
			cur->str=makestr(p,1);
			cur->len=1;
			p++;
			continue;
		}
		error_at(p,"expected a token. char $%02x '%c' found.\t; %s",*p,*p,get_least_line(p));
next_token:;
	}

	// プログラム末
//	printf(";tokenize TK_EOF\n");
	cur = new_token(TK_EOF,cur,"\0",1);
	return head.next;
}

Node *new_node_none()
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NONE;
	node->lhs = NULL;
	node->rhs = NULL;
	node->val = 0;
	node->str = "";

	return	node;
}

Node *new_copy_node(Node *from)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = from->kind;
	node->lhs = from->lhs;
	node->rhs = from->rhs;
	node->val = from->val;
	node->str = from->str;

	return	node;
}

Node *new_node(NodeKind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = node->rhs = new_node_none();
	return node;
}

Node *new_node_string(NodeKind kind,char *str)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->str = str;
	node->lhs = node->rhs = new_node_none();
	return node;
}

Node *new_node_REM(char *str)
{
	return new_node_string(ND_REM,str);
}
Node *new_node_ASM(char *str)
{
	return new_node_string(ND_ASM,str);
}

Node *new_unary(NodeKind kind, Node *lhs)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = new_node_none();
	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	node->rhs = node->lhs = new_node_none();
	return node;
}

Node *new_node_var(char *varname)
{
	Node *node = new_node(ND_VAR);
	node->str = varname;
	node->rhs = node->lhs = new_node_none();
//	printf(";new_node_var varname:%c\n",varname[0]);
	return node;
}
Node *new_node_array1(char *varname,Node *subscripts)
{
	Node *node = new_unary(ND_ARRAY1,subscripts);
	node->str = varname;
//	printf(";new_node_array1 varname:%c\n",varname[0]);
	return node;
}

Node *new_node_array2(char *varname,Node *subscripts)
{
	Node *node = new_unary(ND_ARRAY2,subscripts);
	node->str = varname;
//	printf(";new_node_array2 varname:%c\n",varname[0]);
	return node;
}

int	do_count=0;
int	do_sp=0;
int	do_stack[1000];

//
//	先頭から見つかったDOの数を返す。ラベル作成用
//
int	DO_found()
{
	do_stack[do_sp++] = ++do_count;
	return	do_count;
}
//
//	UNTILに対応するDOラベルを返す
//
int	UNTIL_found()
{
	if(do_sp==0){
		error("UNTIL without DO");
	}
	do_sp--;
	return do_stack[do_sp];
}

int	for_count=0;
int	for_sp=0;
typedef	struct {
	char	*var;
	int		count;
} for_stack_t;
for_stack_t	for_stack[1000];

//
//	先頭から見つかったFORの数を返す。ラベル作成用
//
int	FOR_found(char *str)
{
	for_stack[for_sp].count = ++for_count;
	for_stack[for_sp].var = str;
	for_sp++;
	return	for_count;
}
//
//	NEXTに対応するFORラベルを返す
//
int	NEXT_found(char *str)
{
	while(--for_sp>=0){
		if(strcmp(str,for_stack[for_sp].var)==0){
			return for_stack[for_sp].count;
		}
	}
	error("NEXT %s without FOR",str);
	/* NOT_REACHED */
	return	-1;
}

Node *primary()
{
//	printf(";enter primary()\n");
	// 次のトークンが"("なら、"(" expr ")"のはず
	if (consume_op("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}
	// そうでなければ数値か変数か特殊変数か1文字定数のはず
	Token	*tok;
	if((tok=consume_var())!=NULL){
//		printf(";primary token var or array\n");
		Node	*node;
		if(consume_op(":")){
//			printf(";primary token array1 %s\n",tok->str);
			node = new_node_array1(tok->str,expr());
			expect(")");
		}else if(consume_op("(")){
//			printf(";primary token array2 %s\n",tok->str);
			node = new_node_array2(tok->str,expr());
			expect(")");
		}else{
//			printf(";primary token var %s\n",tok->str);
			node = new_node_var(tok->str);
		}
		return	node;
	}else if((tok=consume_number())!=NULL){
		Node	*node = new_node_num(tok->val);
		return	node;
	}else if(consume_op("&")){		// PG末ポインタ
		Node	*node = new_node(ND_PGEND);
		return	node;
	}else if(consume_op("'")){		// 乱数
		Node	*node = new_unary(ND_RAND,primary());
		return	node;
	}else if(consume_op("%")){		// 剰余
		Node	*node = new_unary(ND_MOD,primary());
		return	node;
	}else if(consume_op("$")){
		Node	*node = new_node(ND_INKEY);
		return	node;
	}else if(token->kind==TK_INKEY){		// 入力
		Node	*node = new_node(ND_INKEY);
		token=token->next;
		return	node;
	}else if(consume_op("?")){		// 入力
		Node	*node = new_node(ND_INPUT);
		return	node;
	}else if(consume_op("\\")){		// TIMER
		Node	*node = new_node(ND_TIMER);
		node->str = "TIMER";
		return	node;
	}else if(consume_op(">")){		// CALL(>=)から戻ってきた時のAccABが入る
		Node	*node = new_node(ND_CALLVAL);
		node->str = "CALLVAL";
		return	node;
	}else if(consume_op("^$")){		// CURSOR ADRS
		Node	*node = new_node(ND_CURSORADRS);
		node->str = "CURSORADRS";
		return	node;
	}else if(consume_op("^")){		// CURSOR ADRS
		Node	*node = new_node(ND_CURSOR);
		node->str = "CURSORADRS";
		return	node;
	}else if(token->kind==TK_KEYBOARD){		// KEYBOARD
		Node	*node = new_node(ND_KEYBOARD);
		node->str = "KEYBOARD";
		token=token->next;
		return	node;
	}else if(token->kind==TK_STR){
//		printf("; ");print_token(token);printf("\n");
		if(token->len!=1){
			error("; illegal string length %d",token->len);
			return	NULL;
		}
		Node	*node = new_node_num(*(token->str));
		token=token->next;
		return	node;
	}
	printf(";primary error next token('%s')=",token->str);print_token(token);
	printf("; at '%s'\n",current_linetop);
	error("terminate compile.");
	return NULL;
}

Node *unary()
{
//	printf(";enter unary()\n");
	if (consume_op("+")){
		Node *node=new_unary(ND_ABS, primary());
		if(isNUM(node->lhs)){
			return new_node_num(abs(node->lhs->val));
		}
		return	node;
	}else if (consume_op("-")){
		Node *node=new_unary(ND_NEG, primary());
		if(isNUM(node->lhs)){
			return new_node_num(-(node->lhs->val));
		}
		return	node;
	}
	return primary();
}


Node *expr()
{
//	printf(";enter expr()\n");
	Node *node = unary();

	for (;;) {
		if(consume_op(">=")){
			node = new_binary(ND_GE, node, unary());
		}else if(consume_op("=>")){
			node = new_binary(ND_GE, node, unary());
		}else if(consume_op("<=")){
			node = new_binary(ND_LE, node, unary());
		}else if(consume_op("=<")){
			node = new_binary(ND_LE, node, unary());
		}else if(consume_op("<>")){
			node = new_binary(ND_NE, node, unary());
		}else if(consume_op("=")){
			node = new_binary(ND_EQ, node, unary());
		}else if(consume_op("<")){
			node = new_binary(ND_LT, node, unary());
		}else if(consume_op(">")){
			node = new_binary(ND_GT, node, unary());
		}else if (consume_op("+")){
			node = new_binary(ND_ADD, node, unary());
		}else if(consume_op("-")){
			node = new_binary(ND_SUB, node, unary());
		}else if(consume_op("*")){
			node = new_binary(ND_MUL, node, unary());
		}else if(consume_op("/")){
			node = new_binary(ND_DIV, node, unary());
		}else if(consume_op("&")){
			node = new_binary(ND_BITAND, node, unary());
		}else if(consume_op(".")){
			node = new_binary(ND_BITOR, node, unary());
		}else if(consume_op("!")){
			node = new_binary(ND_BITXOR, node, unary());
		}else{
		  return node;
		}
	}
}

Node	*assign()
{
//	printf(";assign\n");
	Token	*tok = consume_var();
	Node	*node;
//	printf(";assign: tok->str='%s'\n",token2str(tok));
	if(tok){
//		printf("; check var or array\n");
		if(consume_op(":")){
//			printf("; assign array1 %s\n",tok->str);
			node = new_node_array1(tok->str,expr());
			expect(")");
		}else if(consume_op("(")){
//			printf("; assign array2 %s\n",tok->str);
			node = new_node_array2(tok->str,expr());
			expect(")");
		}else{
//			printf("; assign var %s\n",tok->str);
			node = new_node_var(tok->str);
		}
		if(consume_op("=")){
//			printf("; assign right expression to var/array\n");
			Node *node2 = new_binary(ND_ASSIGN, node, expr());
			if(consume_op(",")){	// FOR
//				printf("; may be for :");print_nodes(node2);
				if(!isVAR(node)){
					error("FOR with ARRAY variable\n");
				}
				Node *node3 = new_binary(ND_FOR,node2,expr());
				node3->str = node->str;
				node3->val = FOR_found(node->str);
//				printf("; ND_FOR %s :",node->str);print_nodes(node3);
				return node3;
			}
//			printf("; may be assign var/array, not FOR\n");
			if(isVAR(node)){	// simple assign VAR=expr
//				printf("; simple assign right expression to var %s\n",node->str);fflush(stdout);
//				printf("; old node2:");print_nodes_ln(node2);
				node2 = new_unary(ND_SETVAR,node2->rhs);
				node2->str = node->str;
//				printf("; new node2:");print_nodes_ln(node2);
//				printf("; simple assign %s= ",node->str);print_nodes_ln(node2);fflush(stdout);
				return node_opt(node2);
//				return node2;
			}
//			printf("; simple assign array/array2\n");
//			printf("; node2: ");print_nodes_ln(node2);
			return	node2;
		}
		printf("; token:");print_token(token);printf("\n");
		error("assign error. '=' not found\n; %s\n",current_linetop);
	}
	printf("; token:");print_token(token);printf("\n");
	error("assign error. no token\n; %s\n",current_linetop);
	return	NULL;
}

typedef	struct	{
	TokenKind	tk;
	NodeKind	nk;
} tknd_pair_t;

tknd_pair_t pseudo_assign1[] = {
	{	TK_IF,				ND_IF			},
	{	TK_SETRAM,			ND_SETRAM		},
	{	TK_PRINT,			ND_PRINT		},
	{	TK_PRINTR,			ND_PRINTR		},
	{	TK_PRINTHEX4,		ND_PRINTHEX4	},
	{	TK_PRINTHEX2,		ND_PRINTHEX2	},
	{	TK_PRINTTAB,		ND_PRINTTAB		},
	{	TK_PRINTCH,			ND_PRINTCH		},
	{	TK_UNTIL,			ND_UNTIL		},
	{	TK_NEXT,			ND_NEXT			},
	{	TK_SETRAND,			ND_SETRAND		},
	{	TK_SETTIMER,		ND_SETTIMER		},
	{	TK_SETCURSOR,		ND_SETCURSOR	},
	{	TK_NONE,			ND_NONE			},
};
tknd_pair_t pseudo_assign2[] = {				// must be number
	{	TK_GOTO,			ND_GOTO			},
	{	TK_GOSUB,			ND_GOSUB		},
};


Node	*stmt()
{
	Node	*node;
//	printf(";stmt start\t");print_token(token);
	for(tknd_pair_t *pa=pseudo_assign1; pa->tk!=TK_NONE; pa++){
//		printf(";pseudo check1 TK_%d==%d\n",pa->tk,token->kind);
		if(consume(pa->tk)){
			Node *ex = expr();
//			printf(";stmt pseudo assign1 ex=");print_nodes(ex);printf("\n");
			node = new_unary(pa->nk,ex);
			if(pa->nk==ND_UNTIL){
//				printf("; may be UNITIL\n");
				node->val=UNTIL_found();
			}else if(pa->nk==ND_NEXT){
//				printf("; may be NEXT\n");
				if(ex->lhs!=NULL && !isVAR(ex->lhs)){
					printf("; NEXT has no control variable:");print_nodes_ln(ex);
					node = new_node(ND_NOP);
				}else{
					node->str=ex->lhs->str;
					node->val=NEXT_found(node->str);
				}
			}else if(pa->nk==ND_PRINTR){
//				printf("; ND_PRINTR\n");
//				printf("; ");print_nodes_ln(ex);
				if(!consume_op(")")){
					error("; ND_PRINTR ')' not found");
				}
				if(!consume_op("=")){
					error("; ND_PRINTR '=' not found");
				}
				node = new_binary(pa->nk,ex,expr());
//				printf("; ND_PRINTR made.\n");
//				printf("; ");print_nodes_ln(node);
			}else if(pa->nk==ND_IF){			// IF .. GOTO 処理
//				printf(";next token:");print_token(token);
//				printf("; ");print_token(token->next);
//				printf("; ");print_token(token->next->next);
//				printf("; ");print_token(token->next->next->next);
				if(token->kind==TK_SEP && token->next->kind==TK_GOTO && token->next->next->kind==TK_NUM){
//					printf("; IF GOTO fusion   ");print_nodes_ln(node);
					int lineno=token->next->next->val;
					node->kind = ND_IFGOTO;
					node->val  = lineno;
					token=token->next->next->next;
//					printf("; => IF GOTO fusion");print_nodes_ln(node);
					if(lineno>0 && (existLINENO(lineno)==NULL)){
						error("; IFGOTO undefined linenumber %d\n",lineno);
					}
					return node;
				}
			}
//			printf(";stmt pseudo assign1 ");print_nodes(node);printf("\n");
//			printf(";stmt pseudo assign1 =");print_nodes(ex);printf("\n");
			return	node;
		}
	}
	for(tknd_pair_t *pa=pseudo_assign2; pa->tk!=TK_NONE; pa++){
//		printf(";pseudo check TK_%d==%d\n",pa->tk,token->kind);
		if(consume(pa->tk)){
			int		sign=1;
			if(consume_op("-")){
				sign=-1;
//				printf("; GOTO/GOSUB -NUM\n");
			}
			Token	*tk=token;
			if(tk->kind==TK_NUM){
				Node *ex = new_node_num(tk->val);
//				printf(";stmt pseudo assign2 ex=");print_nodes(ex);printf("\n");
				node = new_unary(pa->nk,ex);
				node->val = (sign<0)? -1:tk->val;
//				printf(";stmt pseudo assign2 ");print_nodes(node);printf("\n");
//				printf(";stmt pseudo assign2 =");print_nodes(ex);printf("\n");
				if(node->val>0 && existLINENO(node->val)==NULL){
					error("; GOTO/GOSUB undefined line number %d\n",node->val);
				}
				token=token->next;
				return	node;
			}
			error("; number expected");
		}
	}
	if(token->kind==TK_STR){
//		printf(";stmt TK_STR len=%d, '%s'\t",token->len,token->str);
		node = new_node_string(ND_PRINTSTR,token->str);
		token=token->next;
		return	node;
	}
	if(consume(TK_RETURN)){
		return node = new_node(ND_RETURN);
	}
	if(consume(TK_DO)){
		node = new_node(ND_DO);
		node->val = DO_found();
		return node;
	}
	if(consume(TK_PRINTCR)){
		return node = new_node(ND_PRINTCR);
	}
	if(token->kind==TK_REM){
//		printf("; TK_REM '%s'\n",token->str);
		node = new_node_REM(token->str);
		token=token->next;
		return node;
	}
	if(token->kind==TK_ASM){
//		printf("; TK_ASM '%s'\n",token->str);
		node = new_node_ASM(token->str);
		token=token->next;
		return node;
	}
	if(consume(TK_TIMER)){	// タイマーへの代入か?
		if(consume_op("=")){
			node = new_unary(ND_SETTIMER,expr());
			node->str = "TIMER";
			return node;
		}
		error("TIMER '=' not found");
	}
	if(consume(TK_CURSOR)){	// カーソルーへの代入か?
		if(consume_op("=")){
			node = new_unary(ND_SETCURSOR,expr());
			node->str = "CURSOR";
			return node;
		}
		error("CURSOR '=' not found");
	}
	if(token->kind==TK_MUSIC){	// MUSIC
		node=new_node(ND_MUSIC);
		node->str=token->str;
		token=token->next;
		return	node;
	}
	if(consume_op(">=")) {		// 機械語CALL
		node=new_unary(ND_CALL,expr());
		node->str="CALL";
		return	node;
	}
	// その他は代入のはず
	return node = assign();
}

void program()
{
	int	i=0;

	current_linetop = user_input;
//	printf(";progtam start\n");
	int	linenumber = 0;
	int	old_linenumber = 0;
	while(!at_eof()){
//		printf(";line start\t");print_token(token);
		if(token->kind==TK_LINENUM){
			linenumber = token->val;
			if(linenumber<=old_linenumber){
				error("line number error %d,%d\n",linenumber,old_linenumber);
			}
			old_linenumber = linenumber;
		}
		if(token->kind==TK_RESERVED){
//			printf("; ?TK_RESERVED '%s'\n",token->str);
		}
		if(token->kind==TK_EOL){
			token=token->next;
			continue;
		}
		if(token->kind==TK_EOF){
			break;
		}
		if(token->kind!=TK_LINENUM){
			printf("; error at line %d\n",linenumber);
			printf("; token=");print_token(token);
			error("no line number. kind=%d\n",token->kind);
			exit(1);
		}
		current_linetop = token->str;
		Node *node = new_node(ND_LINENUM);
		node->val = token->val;
		node->str = token->str;
		code[i++] = node;
		token=token->next;
		do{
			if(consume_sep()){
				continue;
			}
			if(token->kind==TK_EOL
			|| token->kind==TK_EOF){
				break;
			}
			code[i++] = stmt();
		}while(consume_sep());
//		printf(";line end\n");
		consume_eof();
	}
//	printf(";progtam end\n");
	code[i] = NULL;
}

void
print_program()
{
	for(int	i=0; code[i]!=NULL; i++){
		printf("; ");
		print_nodes(code[i]);
		printf("\n");
	}
}
