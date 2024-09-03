#include	<ctype.h>
#include	<stdarg.h>
#include	<stdbool.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<string.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/stat.h>

//
//	一部のエミュレーターでは動作に非互換がある
//			CPX のフラグの扱い
//
#define		EMU_CPU_BUGS	(1)


typedef	enum	{
	TK_NONE,		//	0
	TK_EOF,			//		EOF
	TK_EOL,			//		行末
	TK_LINENUM,		//
	TK_SEP,			//		文の区切り(SPACE)
	TK_RESERVED,	//	5	記号
	TK_LVAR,		//		変数
	TK_NUM,			//		整数トークン
	TK_IF,			//		';='
	TK_GOTO,		//		'#='
	TK_GOSUB,		//	10	'!='
	TK_RETURN,		//		return ']'
	TK_SETRAM,		//
	TK_PRINT,		//
	TK_PRINTR,		//
	TK_PRINTHEX4,	//	15
	TK_PRINTHEX2,	//
	TK_PRINTTAB,	//
	TK_PRINTCR,		//
	TK_PRINTCH,		//
	TK_STR,			//	20
	TK_CALL,		//
	TK_CALLVAL,		//
	TK_UNTIL,		//
	TK_DO,			//
	TK_NEXT,		//
	TK_FOR,			//
	TK_SETRAND,		//
	TK_INKEY,		//
	TK_TIMER,		//
	TK_SETTIMER,	//
	TK_CURSOR,		//
	TK_SETCURSOR,	//
	TK_CURSORADRS,	//
	TK_SETCURSORADRS,	//
	TK_KEYBOARD,	//
	TK_MUSIC,		//
	TK_MOD,			//
	TK_REM,			//
	TK_ASM,			// assembler extension
} TokenKind;
typedef	struct	Token	Token;

struct	Token {
	TokenKind	kind;	// トークンの型
	Token		*next;	// 次の入力トークン
	int			val;	// kindがTK_NUMの場合その数値
	char		*str;	// トークン文字列/変数名
	int			len;	// トークンの長さ
};



Token	*token;			// 現在注目しているトークン

extern	char *user_input;
extern	char *source_file_name;

// 抽象構文木のノードの種類
typedef enum {
	ND_NONE,	//
	ND_CELL,	//
	ND_NOP,		//
	ND_ADD,		//	+	
	ND_SUB,		//	-
	ND_MUL,		//	*
	ND_DIV,		//	/
	ND_ABS,		//	+ 絶対値(単項+)
	ND_NEG,		//	- 単項
	ND_EQ,		//	=
	ND_NE,		//	!=
	ND_LT,		//	<
	ND_LE,		//	<= 
	ND_GT,		//	>
	ND_GE,		//	>= 
	ND_VAR,		//	変数（左辺値）
	ND_ARRAY1,	//	配列（左辺値）
	ND_ARRAY2,	//	配列（左辺値）
	ND_NUM,		//	整数
	ND_ASSIGN,	//	代入
	ND_IF,		//	;=
	ND_GOTO,	//	u=
	ND_GOSUB,	//	!=
	ND_RETURN,	//	]
	ND_LINENUM,	//
	ND_SETRAM,
	ND_PRINT,
	ND_PRINTR,
	ND_PRINTHEX4,
	ND_PRINTHEX2,
	ND_PRINTTAB,
	ND_PRINTCR,
	ND_PRINTCH,
	ND_PRINTSTR,
	ND_CALL,
	ND_CALLVAL,
	ND_UNTIL,
	ND_DO,
	ND_NEXT,
	ND_FOR,
	ND_SETRAND,
	ND_INKEY,	// $
	ND_INPUT,	// ?
	ND_STR,
	ND_REM,
	ND_RAND,	// '
	ND_MOD,		// %
	ND_PGEND,	// &
	ND_TIMER,	//	0x5c
	ND_SETTIMER,
	ND_CURSOR,	//	^
	ND_SETCURSOR,
	ND_CURSORADRS,	//	$^
	ND_SETCURSORADRS,
	ND_KEYBOARD,	//
	ND_BITAND,	//
	ND_BITOR,	//
	ND_BITXOR,	//
	ND_MUSIC,	//
//	以下オプティマイズ用
	ND_SETVAR,
	ND_SETVAR_N,
	ND_SETARY1_N,
	ND_SETARY2_N,
	ND_ASLD,
	ND_ANDI,
	ND_ANDI_MOD,
	ND_NEGVAR,
	ND_INCVAR,
	ND_DECVAR,
	ND_INC2VAR,
	ND_DEC2VAR,
	ND_ASM,
	ND_IFGOTO,
	ND_STACKTOP,
	ND_RELMUL,	//	関係演算同士の*
	ND_RELADD,	//	関係演算同士の+
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind;	// ノードの型
	Node *lhs;		// 左辺
	Node *rhs;		// 右辺
	int val;		// kindがND_NUM
	char *str;;		// kindがND_STR,ND_VAR
};

//
// from parse.c
//
extern	int		for_count;
void	print_token(Token *token);
void	print_nodes(Node *node);
void	print_nodes_ln(Node *node);
void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);
Node	*new_node_none();
Node	*new_node(NodeKind kind);
Node	*new_copy_node(Node *from);
Node	*new_node_num(int val);
Node	*new_node_var(char *varname);
Node	*new_node_string(NodeKind kind,char *str);
Node	*new_node_REM(char *str);
Node	*new_unary(NodeKind kind, Node *lhs);
Node	*new_binary(NodeKind kind, Node *lhs, Node *rhs);
bool	consume(TokenKind tk);
void	expect(char *op);
int		expect_number();
Token	*tokenize();
Token *new_token(TokenKind kind, Token *cur, char *str,int len);
void program();
void print_program();
char *existLINENO(int v);
int usedLINENO(int v);
//
// from optimize.c
//
int		isNUM(Node *node);
int		isVAR(Node *node);
int		isNUMorVAR(Node *node);
int		isCompare(Node *node);
int		isEQorNE(Node *node);
int		isSameNUM(Node *x,Node *y);
int		isSameVAR(Node *x,Node *y);
int		isARRAY1(Node *node);
int		isARRAY2(Node *node);
int		isARRAY(Node *node);
int		isSameARRAY(Node *x,Node *y);
int		isADDorSUB(Node *node);
int		has_side_effect(Node *node);
Node	*node_opt(Node *node);
void	multi_statement_optimize();
void	optimize_for_loop();

typedef	struct	{
		char		*var;			// loop counter
		int			opt;			// can optimize? (0:No, !0:Yes)
		int			n;				// number of arrays
		NodeKind	type[100];		// ND_ARRAY1 or ND_ARRAY2
		char		*arrays[100];	// loop内の配列。array:var+offset), array(var+offset)のみ
		char		*label[100];
} opt_for_loop_t;
opt_for_loop_t	ofl[1000];
extern	int	ofl_n;

//
// from codegen.c
//
Node *expr();
void gen_stmt(Node *node);
void prologue();
void epilogue();

//
//	from bmchar.c
//
char	*escape_bmchar(char *str);

Node	*code[10000];
typedef	struct {
		char	*label;
		char	*str;				// bmstring
		char	*orig;				// unicode
} print_string_t;
print_string_t	STRING_FCC[10000];

