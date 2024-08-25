#include	"common.h"

int	string_count;
typedef	struct {
	int		type;	// ND_NUM or ELSE
	int		val;	// ND_NUMの時の終値
	char	*var;	// FOR変数名（debug用)
	char	*loop;	// ループ戻り先ラベル
	char	*to;	// 終値格納先ラベル
} forto_t;
forto_t	FORTO[1000];

typedef struct	{
	int		valid;
	int		val;
	int		n;
	char	*var[256];
	int		offset;
	char	*v_off;
} loc_t;
loc_t	loc_AA;
loc_t	loc_BB;
loc_t	loc_DD;
loc_t	loc_XX;
loc_t	*loc_A = &loc_AA;
loc_t	*loc_B = &loc_BB;
loc_t	*loc_D = &loc_DD;
loc_t	*loc_X = &loc_XX;
typedef	struct {
	int		scale;	// if scale==0, invalid data. scale hold 1 or 2
	char	*v1;	// base variable
	char	*v2;	// offset var
} tdxwk_t;
tdxwk_t	loc_TT;
tdxwk_t	*loc_TDXWK = &loc_TT;

void purge_loc(loc_t *p)
{
#if	0
	printf("; purge_loc ");
	if(p==loc_A){
		printf("A\n");
	}else if(p==loc_B){
		printf("B\n");
	}else if(p==loc_D){
		printf("D\n");
	}else if(p==loc_X){
		printf("X\n");
	}
#endif
	p->valid = 0;
	p->n = 0;
	p->offset = 0;
}
int	is_free_loc_A()
{
	loc_t	*p = loc_A;
	return (p->valid==0 && p->n==0 && p->offset==0);
}
int	is_free_loc_B()
{
	loc_t	*p = loc_B;
	return (p->valid==0 && p->n==0 && p->offset==0);
}
int	is_free_loc_X()
{
	loc_t	*p = loc_X;
	return (p->valid==0 && p->n==0 && p->offset==0);
}

void purge_loc_B()
{
	purge_loc(loc_B);
	purge_loc(loc_D);
}
void purge_loc_A()
{
	purge_loc(loc_A);
	purge_loc(loc_D);
}
void purge_loc_D()
{
	purge_loc(loc_A);
	purge_loc(loc_B);
	purge_loc(loc_D);
}
void purge_loc_X()
{
	purge_loc(loc_X);
}
void	purge_loc_var_sub(loc_t *p, char *v);

void purge_loc_TDXWK()
{
#if	0
	printf("; purge loc TDXWK called\n");
	if(loc_TDXWK->scale){
		tdxwk_t *p=loc_TDXWK;
		printf("; purge loc TDXWK %s+%s*%d\n",p->v1,p->v2,p->scale);
	}
#endif
	loc_TDXWK->scale = 0;
	purge_loc_var_sub(loc_D,"TDXWK");
	purge_loc_var_sub(loc_X,"TDXWK");
}
void purge_loc_all()
{
	purge_loc(loc_A);
	purge_loc(loc_B);
	purge_loc(loc_D);
	purge_loc(loc_X);
	purge_loc_TDXWK();
}
void add_loc_const(loc_t *p,int n)
{
	purge_loc(p);
	p->valid = 1;
	p->val = n;
}
void add_loc_const_A(int n)
{
	n = n&0x0ff;
	add_loc_const(loc_A,n);
	if(loc_B->valid){
		loc_D->valid=1;
		loc_D->val = (loc_A->val<<8)+loc_B->val;
		return;
	}
	purge_loc(loc_D);
}
void add_loc_const_B(int n)
{
	n = n&0x0ff;
	add_loc_const(loc_B,n);
	if(loc_A->valid){
		loc_D->valid=1;
		loc_D->val = (loc_A->val<<8)+loc_B->val;
		return;
	}
	purge_loc(loc_D);
}
void add_loc_var(loc_t *p,char *s)
{
	p->var[p->n++] = s;
#if	0
	if(p==loc_X){
		printf("; add_loc_var %s to IX\n",s);
		printf("; =>");
		for(int i=0; i<p->n; i++){
			printf(" %s",p->var[i]);
		}
		if(p->valid){
			printf(", %d",p->val);
		}
		printf("\n");
	}
#endif
}
void add_loc_var_offset(loc_t *p,char *s,int v)
{
	purge_loc(p);
	p->v_off = s;
	p->offset = v;
}
void add_loc_TDXWK_var(int scale,char *v1,char *v2){		// Var(var)
	tdxwk_t *p = loc_TDXWK;
	p->scale = scale;
	p->v1 = v1;
	p->v2 = v2;
//	printf("; add loc TDXWK for %s+%s*%d\n",v1,v2,scale);
}
int	exist_loc_TDXWK(int scale,char *v1,char *v2)
{
	tdxwk_t *p = loc_TDXWK;
#if	0
	printf("; search TDXWK for %s+%s*%d",v1,v2,scale);
	if ((p->scale==scale)
		&&  (strcmp(p->v1,v1)==0)
		&&  (strcmp(p->v2,v2)==0)){
		printf(" => found\n");
	}else{
		printf(" => not found\n");
	}
#endif
	return ((p->scale==scale)
		&&  (strcmp(p->v1,v1)==0)
		&&  (strcmp(p->v2,v2)==0));
}

int exist_loc_var_offset(loc_t *p,char *s,int v)
{
	if(p->offset!=0
	&& p->offset==v
	&& strcmp(p->v_off,s)==0){
		return 1;
	}
	return 0;
}
int exist_loc_const(loc_t *p,int n)
{
	if(p->valid && p->val==n){
		return	1;
	}
	return 0;
}
int exist_loc_const_A(int n)
{
	n = n&0x0ff;
	return	exist_loc_const(loc_A,n);
}
int exist_loc_const_B(int n)
{
	n = n&0x0ff;
	return	exist_loc_const(loc_B,n);
}
int exist_loc_const_X(int n)
{
	return	exist_loc_const(loc_X,n);
}

int exist_loc_var(loc_t *p,char *s)
{
	if(p->n==0) return 0;
	for(int i=0; i<p->n; i++){
		if(p->var[i]!=NULL && strcmp(p->var[i],s)==0) return 1;
	}
	return 0;
}

void purge_loc_var_sub(loc_t *p,char *s)
{
	if(p->offset!=0 && strcmp(p->v_off,s)==0){
		p->offset = 0;
	}
	if(p->n==0) return; 
	for(int i=0; i<p->n; i++){
		if(p->var[i]!=NULL && strcmp(p->var[i],s)==0) p->var[i]=NULL;
	}
}
void purge_loc_var_TDXWK(char *s)
{
	tdxwk_t *p = loc_TDXWK;
	if(p->scale==0){		// 使ってない
		return;
	}
	if(strcmp(p->v1,s)==0){	// 配列変数がsだった
		p->scale = 0;
	}if(strcmp(p->v2,s)==0){// 添字にsが使われていた
		p->scale = 0;
	}
	purge_loc_var_sub(loc_X,"TDXWK");
	return;
}
void purge_loc_var(char *s)
{
	purge_loc_var_sub(loc_B,s);
	purge_loc_var_sub(loc_D,s);
	purge_loc_var_sub(loc_X,s);
	purge_loc_var_TDXWK(s);
}
void	print_line(char *p)
{
	while(*p==' '){
		p++;
	}
	while(*p!='\n' && *p!='\r' && *p!='\0'){
		putchar(*p++);
	}
}

char	*
new_line_label(int v)
{
	char	*label = calloc(1,10);

	sprintf(label,".LN%d",(v>=0)?v:65536);
	return	label;
}

char	*
new_label()
{
	static	uint16_t	labelnum = 0;
	char	*label = calloc(1,10);

	sprintf(label,".LL%d",labelnum++);
	return	label;
}

char	*
new_do_label(int v)
{
	char	*label = calloc(1,10);

	sprintf(label,".LD%d",v);
	return	label;
}
char	*
new_for_label(int v)
{
	char	*label = calloc(1,20);

	sprintf(label,".LF%d",v);
	return	label;
}
char	*
new_to_label(int v)
{
	char	*label = calloc(1,20);

	sprintf(label,".LT%d",v);
	return	label;
}

void
LABEL(char *label)
{
	printf("%s\tEQU\t*\n",label);
}

uint8_t	low(int x)
{
	return	(x & 0x0ff);
}

uint8_t	high(int x)
{
	return	((x & 0x0ff00)>>8);
}


void	TSTA()
{
		printf("\tTSTA\n");
}
void	TSTB()
{
		printf("\tTSTB\n");
}
void	TST_X(int v)
{
		printf("\tTST\t%d,X\n",v);
}
void	INCA()
{
		printf("\tINCA\n");
		if(loc_A->valid){
			int	v=loc_A->val+1;
			purge_loc(loc_A);
			add_loc_const_A(v&0x00ff);
			return;
		}
		purge_loc_A();
}
void	INCB()
{
		printf("\tINCB\n");
		if(loc_B->valid){
			int	v=loc_B->val+1;
			purge_loc(loc_B);
			add_loc_const_B(v&0x00ff);
			return;
		}
		purge_loc_B();
}
void	DECA()
{
		printf("\tDECA\n");
		if(loc_A->valid){
			int	v=loc_A->val-1;
			purge_loc(loc_A);
			add_loc_const_A(v&0x00ff);
			return;
		}
		purge_loc_A();
}
void	DECB()
{
		printf("\tDECB\n");
		if(loc_B->valid){
			int	v=loc_B->val-1;
			purge_loc(loc_B);
			add_loc_const_B(v&0x00ff);
			return;
		}
		purge_loc_B();
}
void	INC_V0(char *v)
{
		printf("\tINC\t_%s\n",v);
		purge_loc_var(v);
}
void	DEC_V0(char *v)
{
		printf("\tDEC\t_%s\n",v);
		purge_loc_var(v);
}
void	INC_V1(char *v)
{
		printf("\tINC\t_%s+1\n",v);
		purge_loc_var(v);
}
void	DEC_V1(char *v)
{
		printf("\tDEC\t_%s+1\n",v);
		purge_loc_var(v);
}
void	TST_V0(char	*str)
{
		printf("\tTST\t_%s\n",str);
}
void	TSX()
{
		printf("\tTSX\n");
		purge_loc_X();
}
void	INX()
{
		printf("\tINX\n");
		if(loc_X->valid){
			int	val = loc_X->val+1;
			purge_loc(loc_X);
			loc_X->valid = 1;
			loc_X->val = val;
			return;
		}
		purge_loc(loc_X);
}
void	INX2()
{
		INX();
		INX();
}
void	DEX()
{
		printf("\tDEX\n");
		if(loc_X->valid){
			int	val = loc_X->val-1;
			purge_loc(loc_X);
			loc_X->valid = 1;
			loc_X->val = val;
			return;
		}
		purge_loc(loc_X);
}
void	DEX2()
{
		DEX();
		DEX();
}
void	INS()
{
		printf("\tINS\n");
}
void	INS2()
{
		INS();
		INS();
}

void	PSHA()
{
		printf("\tPSHA\n");
}
void	PSHB()
{
		printf("\tPSHB\n");
}
void	PSHD()
{
		PSHB();
		PSHA();
}
void	PULA()
{
		printf("\tPULA\n");
		purge_loc_A();
}
void	PULB()
{
		printf("\tPULB\n");
		purge_loc_B();
}
void	PULD()
{
		PULA();
		PULB();
}
void	ASLD()
{
		printf("\tASLB\n");
		printf("\tROLA\n");
		purge_loc_D();
}
void	ASLD_N(int n)
{
		while(n-->0){
			printf("\tASLB\n");
			printf("\tROLA\n");
		}
		purge_loc_D();
}
void	ASRD_N(int n)
{
		while(n-->0){
			printf("\tASRA\n");
			printf("\tRORB\n");
		}
		purge_loc_D();
}

void	NEGD()
{
		printf("\tNEGA\n");
		printf("\tNEGB\n");
		printf("\tSBCA\t#0\n");
		purge_loc_D();
}

void Bxx(char *cc, char *label);

void	NEG_V(char *v)
{
		char	*label = new_label();
		printf("\tNEG\t_%s+1\n",v);
		Bxx("NE",label);
		printf("\tDEC\t_%s\n",v);
		printf("%s\tCOM\t_%s\n",label,v);
		purge_loc_var(v);
}

void	ABSD()
{
		char	*positive  = new_label();
		TSTA();
		Bxx("PL",positive);
		NEGD();
		LABEL(positive);
		purge_loc_D();
}

void	TAB()
{
		printf("\tTAB\n");
		purge_loc_B();
		if(loc_A->valid){
			add_loc_const_B(loc_A->val);
		}
}
void	TBA()
{
		printf("\tTBA\n");
		purge_loc_A();
		if(loc_B->valid){
			add_loc_const_A(loc_B->val);
		}
}
void	CLRA()
{
		if(exist_loc_const_A(0)){
			printf("; CLRA optimized\n");
			return;
		}
		printf("\tCLRA\n");
		add_loc_const_A(0);
}
void	CLRB()
{
		if(exist_loc_const_B(0)){
			printf("; CLRD optimized\n");
			return;
		}
		printf("\tCLRB\n");
		add_loc_const_B(0);
}
void	LDAA_I(int v)
{
		if(exist_loc_const_A(v)){
			printf("; LDAA #%d optimized\n",v);
		}else if(v==0){
			CLRA();
		}else if(exist_loc_const_B(v)){
			printf("; LDAA #%d optimized\n",v);
			TBA();
		}else if(exist_loc_const_A(v-1)){
			printf("; LDAA #%d optimized to INCA\n",v);
			INCA();
		}else if(exist_loc_const_A(v+1)){
			printf("; LDAA #%d optimized to DECA\n",v);
			DECA();
		}else{
			printf("\tLDAA\t#%d\n",v);
			add_loc_const_A(v);
		}	
}
void	LDAA_V0(char *str)
{
		printf("\tLDAA\t_%s\n",str);
		purge_loc_A();
}
void	LDAA_V1(char *str)
{
		printf("\tLDAA\t_%s+1\n",str);
		purge_loc_A();
}
void	LDAA_V(char *str)
{
		LDAA_V0(str);
		purge_loc_A();
}
void	LDAA_X(int v)
{
		printf("\tLDAA\t%d,X\n",v);
		purge_loc_A();
}
void	LDAA_IVX(Node *node)
{
		if(isNUM(node)){
			LDAA_I(high(node->val));
		}else if(isVAR(node)){
			LDAA_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			LDAA_X(node->val);
		}else{
			printf("; LDAA_IVX:");print_nodes_ln(node);
			error("; LDAA_IVX ");
		}
}
void	LDAB_I(int v)
{
		if(exist_loc_const(loc_B,v)){
			printf("; LDAB #%d optimized\n",v);
			return;
		}else if(v==0){
			CLRB();
		}else if(exist_loc_const_A(v)){
			printf("; LDAB #%d optimized\n",v);
			TAB();
		}else if(exist_loc_const_B(v-1)){
			printf("; LDAB #%d optimized to INCB\n",v);
			INCB();
		}else if(exist_loc_const_B(v+1)){
			printf("; LDAB #%d optimized to DECB\n",v);
			DECB();
		}else{
			printf("\tLDAB\t#%d\n",v);
			purge_loc_B();
			add_loc_const(loc_B,v);
		}
}
void	LDAB_V(char *str)
{
		if(exist_loc_var(loc_B,str)){
			printf("; LDAB var %s optimized\n",str);
			return;
		}
		printf("\tLDAB\t_%s+1\n",str);
		purge_loc_B();
}
void	LDAB_X(int v)
{
		printf("\tLDAB\t%d,X\n",v);
		purge_loc_B();
}
void	LDAB_IVX(Node *node)
{
		if(isNUM(node)){
			LDAB_I(low(node->val));
		}else if(isVAR(node)){
			LDAB_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			LDAB_X(node->val+1);
		}else{
			printf("; CMPB_IVX:");print_nodes_ln(node);
			error("; CMPB_IVX ");
		}
}
void	LDD_I(int v)
{
		if(exist_loc_const(loc_D,v)){
printf("; loc_D->valid=%d, loc_D->n=%d\n",loc_D->valid,loc_D->n);
			printf("; LDD #%d optimized\n",v);
			return;
		}
		if(v!=0 && high(v)==low(v)){
			LDAB_I(low(v));
			TBA();
			return;
		}
		if(low(v)==0){
			CLRB();
		}else{
			LDAB_I(low(v));
		}
		if(high(v)==0){
			CLRA();
		}else{
			LDAA_I(high(v));
		}
}
void	LDD_IV(char *v)
{
		printf("\tLDAB\t#_%s\n", 	v);
		printf("\tLDAA\t#_%s/256\n",v);
		purge_loc_D();
}
void	LDD_X(int v)
{
		printf("\tLDAB\t%d,X\n",v+1);
		printf("\tLDAA\t%d,X\n",v);
		purge_loc_D();
}
void	LDD_V(char *v)
{
		if(exist_loc_var(loc_D,v)){
			printf("; LDD var %s optimized\n",v);
		}else{
			printf("\tLDAB\t_%s+1\n", v);
			printf("\tLDAA\t_%s\n", v);
		}
		purge_loc_D();
		add_loc_var(loc_D,v);
}
void	LDD_L(char *str)
{
		printf("\tLDAB\t%s+1\n", str);
		printf("\tLDAA\t%s\n", str);
		purge_loc_D();
}
void	LDD_IVX(Node *node)		// SUB # or Var or n,X
{
		if(isNUM(node)){
			LDD_I(node->val);
		}else if(isVAR(node)){
			LDD_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			LDD_X(node->val);
		}else{
			printf("; LDD_IVX:");print_nodes_ln(node);
			error("; LDD_IVX ");
		}
}
void	ABA()
{
		printf("\tABA\n");
		purge_loc_A();
}
void	ADD_I(int v)
{
		printf("\tADDB\t#%d\n", low(v));
		printf("\tADCA\t#%d\n", high(v));
		purge_loc_D();
}
void	ADD_IV(char *v)
{
		printf("\tADDB\t#_%s+1\n",	v);
		printf("\tADCA\t#_%s\n",	v);
		purge_loc_D();
}
void	ADD_V(char *v)
{
		printf("\tADDB\t_%s+1\n",	v);
		printf("\tADCA\t_%s\n",		v);
		purge_loc_D();
}
void	ADD_L(char *str)
{
		printf("\tADDB\t%s+1\n", str);
		printf("\tADCA\t%s\n", str);
		purge_loc_D();
}
void	ADD_X(int v)
{
		printf("\tADDB\t%d,X\n", v+1);
		printf("\tADCA\t%d,X\n", v);
		purge_loc_D();
}
void	SUB_I(int v)
{
		printf("\tSUBB\t#%d\n", low(v));
		printf("\tSBCA\t#%d\n", high(v));
		purge_loc_D();
}
void	SUB_X(int v)
{
		printf("\tSUBB\t%d,X\n", v+1);
		printf("\tSBCA\t%d,X\n", v);
		purge_loc_D();
}
void	SUB_V(char *v)
{
		printf("\tSUBB\t_%s+1\n",	v);
		printf("\tSBCA\t_%s\n",		v);
		purge_loc_D();
}
void	SUB_L(char *v)
{
		printf("\tSUBB\t%s+1\n",	v);
		printf("\tSBCA\t%s\n",		v);
		purge_loc_D();
}
void	SUB_IVX(Node *node)		// SUB # or Var or n,X
{
		if(isNUM(node)){
			SUB_I(node->val);
		}else if(isVAR(node)){
			SUB_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			SUB_X(node->val);
		}else{
			printf("; SUB_IVX:");print_nodes_ln(node);
			error("; SUB_IVX ");
		}
}
//
void	ANDA_I(int v)
{
		printf("\tANDA\t#%d\n",low(v));
		purge_loc_A();
}
void	ANDB_I(int v)
{
		printf("\tANDB\t#%d\n",low(v));
		purge_loc_B();
}
void	AND_I(int v)
{
		ANDB_I(low(v));
		ANDA_I(high(v));
		purge_loc_D();
}
void	AND_X(int v)
{
		printf("\tANDB\t%d,X\n", v+1);
		printf("\tANDA\t%d,X\n", v);
		purge_loc_D();
}
void	AND_V(char *v)
{
		printf("\tANDB\t_%s+1\n",	v);
		printf("\tANDA\t_%s\n",		v);
		purge_loc_D();
}
void	AND_L(char *v)
{
		printf("\tANDB\t%s+1\n",	v);
		printf("\tANDA\t%s\n",		v);
		purge_loc_D();
}
//
void	OR_I(int v)
{
		printf("\tORAB\t#%d\n", low(v));
		printf("\tORAA\t#%d\n", high(v));
		purge_loc_D();
}
void	OR_X(int v)
{
		printf("\tORAB\t%d,X\n", v+1);
		printf("\tORAA\t%d,X\n", v);
		purge_loc_D();
}
void	OR_V(char *v)
{
		printf("\tORAB\t_%s+1\n",	v);
		printf("\tORAA\t_%s\n",		v);
		purge_loc_D();
}
void	OR_L(char *v)
{
		printf("\tORAB\t%s+1\n",	v);
		printf("\tORAA\t%s\n",		v);
		purge_loc_D();
}
//
void	EOR_I(int v)
{
		printf("\tEORB\t#%d\n", low(v));
		printf("\tEORA\t#%d\n", high(v));
		purge_loc_D();
}
void	EOR_X(int v)
{
		printf("\tEORB\t%d,X\n", v+1);
		printf("\tEORA\t%d,X\n", v);
		purge_loc_D();
}
void	EOR_V(char *v)
{
		printf("\tEORB\t_%s+1\n",	v);
		printf("\tEORA\t_%s\n",		v);
		purge_loc_D();
}
void	EOR_L(char *v)
{
		printf("\tEORB\t%s+1\n",	v);
		printf("\tEORA\t%s\n",		v);
		purge_loc_D();
}
void	BITA_I(int v)
{
		printf("\tBITA\t%d\n",v);
}
void	BITB_I(int v)
{
		printf("\tBITB\t%d\n",v);
}

void	CLRD()
{
		CLRB();
		CLRA();
		purge_loc_D();
		add_loc_const(loc_B,0);
		add_loc_const(loc_D,0);
		add_loc_const(loc_D,0);
}
void	CLR_V(char *v)
{
		printf("\tCLR\t_%s+1\n", v);
		printf("\tCLR\t_%s\n", v);
		purge_loc_var(v);
}
void	CLR_V1(char *v)
{
		printf("\tCLR\t_%s+1\n", v);
		purge_loc_var(v);
}
void	CLR_V0(char *v)
{
		printf("\tCLR\t_%s\n", v);
		purge_loc_var(v);
}
void	STD_X(int v);
void	STAA_X(int v);
void	STAB_X(int v);

void	CLR_X(int v)
{
		if(exist_loc_const(loc_D,0)){
			printf("; CLR ,X #%d optimized to STD\n",v);
			STD_X(v);
			return;
		}else if(exist_loc_const(loc_A,0)){
			printf("; CLR ,X #%d optimized to STAA\n",v);
			STAA_X(v+1);				// 6 2
			STAA_X(v);					// 6 2
		}else if(exist_loc_const(loc_B,0)){
			printf("; CLR ,X #%d optimized to STAB\n",v);
			STAB_X(v+1);				// 6 2
			STAB_X(v);					// 6 2
		}
		printf("\tCLR\t%d,X\n",v+1);	// 7 2
		printf("\tCLR\t%d,X\n",v);		// 7 2
}
void	CLR1_X(int v)
{
		printf("\tCLR\t%d,X\n",v);
}
void	CMPA_I(int v)
{
		printf("\tCMPA\t#%d\n",v);
}
void	CMPB_I(int v)
{
		printf("\tCMPB\t#%d\n",v);
}
void	CMPA_X(int v)
{
		printf("\tCMPA\t%d,X\n",v);
}
void	CMPB_X(int v)
{
		printf("\tCMPB\t%d,X\n",v);
}
void	CMPA_V(char *str)
{
		printf("\tCMPA\t_%s\n",str);
}
void	CMPB_V(char *str)
{
		printf("\tCMPB\t_%s+1\n",str);
}
void	CMPA_IVX(Node *node)
{
		if(isNUM(node)){
			CMPA_I(high(node->val));
		}else if(isVAR(node)){
			CMPA_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			CMPA_X(node->val);
		}else{
			printf("; CMPA_IVX:");print_nodes_ln(node);
			error("; CMPA_IVX ");
		}
}
void	CMPB_IVX(Node *node)
{
		if(isNUM(node)){
			CMPB_I(low(node->val));
		}else if(isVAR(node)){
			CMPB_V(node->str);
		}else if(node->kind==ND_STACKTOP){
			CMPB_X(node->val+1);
		}else{
			printf("; CMPB_IVX:");print_nodes_ln(node);
			error("; CMPB_IVX ");
		}
}

void	STD_V(char *str)
{
		printf("\tSTAB\t_%s+1\n",	str);
		printf("\tSTAA\t_%s\n",		str);
		purge_loc_var(str);
		add_loc_var(loc_D,str);
}
void	STD_X(int v)
{
		printf("\tSTAB\t%d,X\n",v+1);
		printf("\tSTAA\t%d,X\n",v);
}
void	STD_L(char *str)
{
		printf("\tSTAB\t%s+1\n",str);
		printf("\tSTAA\t%s\n",	str);
}
void	STAA_X(int v)
{
		printf("\tSTAA\t%d,X\n",v);
}
void	STAB_X(int v)
{
		printf("\tSTAB\t%d,X\n",v);
}
void	STAA_V(char *str)
{
		printf("\tSTAB\t_%s\n",str);
}
void	STAB_V(char *str)
{
		printf("\tSTAB\t_%s+1\n",str);
		purge_loc_var(str);
}
void	LDX_I(int v)
{
		if(exist_loc_const(loc_X,v)){
			printf("; LDX #%d optimized\n",v);
			return;
#if	0
		}else if(exist_loc_const(loc_X,v+1)){			// 2バイト減るけど1サイクル増える。微妙
			printf("; LDX #%d optimized to DEX\n",v);
			DEX();
			return;
		}else if(exist_loc_const(loc_X,v)){
			printf("; LDX #%d optimized to INX\n",v);
			INX();
			return;
#endif
		}
		printf("\tLDX\t#%d\n",v);
		purge_loc_X();
		add_loc_const(loc_X,v);
}
void	LDX_X(int v)
{
		printf("\tLDX\t%d,X\n",v);
		purge_loc_X();
}
void	LDX_V(char *str)
{
#if	0
		if(loc_X->n){
			for(int i=0; i<loc_X->n; i++){
				if(loc_X->var[i]!=NULL && strcmp(loc_X->var[i],str)==0){
					printf("; LDX_V search %s=%s ?\n",str,loc_X->var[i]);
				}
			}
		}
#endif
		if(exist_loc_var(loc_X,str)){
			printf("; LDX var %s optimized\n",str);
			return;
		}
		printf("\tLDX\t_%s\n",str);
		purge_loc_X();
		add_loc_var(loc_X,str);
}
void	LDX_L(char *str)
{
		printf("\tLDX\t%s\n",str);
		purge_loc_X();
}
void	LDX_IL(char *str)
{
		printf("\tLDX\t#%s\n",str);
		purge_loc_X();
}
void	STX_V(char *v)
{
		printf("\tSTX\t_%s\n",	v);
		purge_loc_var(v);
}
void	STX_L(char *v)
{
		printf("\tSTX\t%s\n",	v);
}
void	CPX_I(int v)
{
		printf("\tCPX\t#%d\n",v);
}
void	CPX_V(char *str)
{
		printf("\tCPX\t_%s\n",str);
}
void	CPX_L(char *str)
{
		printf("\tCPX\t%s\n",str);
}
void	LDX_Vp(char *str,int n)
{
		if(exist_loc_var_offset(loc_X,str,n)){
			printf("; LDX var %s+%d optimized\n",str,n);
			return;
		}
		if(abs(n)%256!=0){
			error("; error LDX_vp %s %d\n",str,n);
		}
		if(n>0){
			for(int i=0; i<n; i+=256) INC_V0(str);
			LDX_V(str);
			for(int i=0; i<n; i+=256) DEC_V0(str);
		}else{
			for(int i=0; i<abs(n); i+=256) DEC_V0(str);
			LDX_V(str);
			for(int i=0; i<abs(n); i+=256) INC_V0(str);
		}
		add_loc_var_offset(loc_X,str,n);
}

void	BRA(char *to)
{
		printf("\tBRA\t%s\n",to);
}
void	JMP(char *to)
{
		printf("\tJMP\t%s\n",to);
}
void	JSR(char *to)
{
		printf("\tJSR\t%s\n",to);
		purge_loc_all();
}
void	JSR_X(int v)
{
		printf("\tJSR\t%d,X\n",v);
		purge_loc_all();
}
void	MUL256()
{
		TBA();
		CLRB();
}

void
Bxx(char *cc, char *label)
{
		printf("\tB%s\t%s\n",cc,label);
}

void	RMB_comment(char *label,int size,char *comment)
{
		printf("%s\tRMB\t\%d\t; %s\n",label,size,comment);
}
void	RMB(char *label,int size)
{
		printf("%s\tRMB\t\%d\n",label,size);
}

void	FCC_STR(char *str)
{
		printf("\tFCB\t\"%s\"\n",str);
		printf("\tFCB\t0\n");
}

void	SKIP1()
{
		printf("\tFCB\t$81		; SKIP 1byte\n");
}
void	SKIP2()
{
		printf("\tFCB\t$8C		; SKIP 2byte\n");
}

//		LEAX	D,X
void	ADX()
{
#if		1
		JSR("ADX");
		purge_loc_all();
#else
		STX_L("ADXWK");
		ADD_L("ADXWK");
		STD_L("ADXWK");
		LDX_L("ADXWK");
#endif
}
//		LEAX	D*2,X
void	ADX2()
{
#if		1
		JSR("ADX2");
		purge_loc_all();
#else
		STX_L("ADXWK");
		ASLD();
		ADD_L("ADXWK");
		STD_L("ADXWK");
#endif
}
//		TFR	D,X
void	TDX()
{
#if		1
		// 12cycle 6bytes (with DP)
		purge_loc_TDXWK();
		STD_V("TDXWK");
		purge_loc_X();
		LDX_V("TDXWK");
#else
		// 26cycle 7bytes
		PSHB();
		PSHA();
		TSX();
		LDX_X(0);
		INS2();
#endif
}

void	print_debug(char *str)
{
		char	*label = new_label();
		LDX_IL(label);
		JSR("PRINTSTR");
		JSR("PRINTCR");
		STRING_FCC[string_count].label = label;
		STRING_FCC[string_count].str   = escape_bmchar(str);
		STRING_FCC[string_count].orig  = str;
		string_count++;
}

void	gen_save(char *save)
{
	if(strchr(save,'D')!=NULL){
		PSHD();
	}else if(strchr(save,'A')!=NULL){
		PSHA();
	}else if(strchr(save,'B')!=NULL){
		PSHB();
	}
}
void	gen_restore(char *save)
{
	if(strchr(save,'D')!=NULL){
		PULD();
	}else if(strchr(save,'A')!=NULL){
		PULA();
	}else if(strchr(save,'B')!=NULL){
		PULB();
	}
}

void	gen_store_var(Node *node)
{
		STD_V(node->str);
}

void gen_ARRAY1(Node *node);
void gen_expr(Node *node);

//
//	var:offset) または var(offset)のアドレスを得る(IX+戻り値)
//		ex. A:1)
//				IX = _A
//				return 1
//		ex. A(1)
//				IX = _A
//				return 2
//		ex. A(V+1)
//				IX = _A + _V*2
//				return 2
//		save: 保存しないといけないレジスタ
//				"A" or "B" or "D"
//
int
gen_array_address(Node *node,char *save)
{
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; gen_array_address error\n");
	}
//	printf("; gen_array_address:");print_nodes_ln(node);
	if(node->kind==ND_ARRAY1){						// 1バイト間接モード
		if(isNUM(node->lhs)){					// 添字が定数
			int offset = node->lhs->val;
			if(offset>=0 && offset<256){
				LDX_V(node->str);
				return offset;
#ifdef	O_SPEED
			}else if(offset<0 && offset>=-3){
#else
			}else if(offset<0 && offset>=-6){
#endif
				LDX_V(node->str);
				switch(offset){
				case -6: DEX();
				case -5: DEX();
				case -4: DEX();
				case -3: DEX();
				case -2: DEX();
				case -1: DEX();
						return	0;
				default: break;
				}
			}else if(offset>=256 && offset<512){
				LDX_Vp(node->str,256);
				return	offset-256;
#ifndef	O_SPEED
			}else if(offset>=512 && offset<1024){
				LDX_Vp(node->str,512);
				return	offset-512;
#endif
			}else if(offset<0 && offset>=-256){
				LDX_Vp(node->str,-256);
				return	offset+256;
#ifndef	O_SPEED
			}else if(offset<-256 && offset>=-512){
				LDX_Vp(node->str,-512);
				return	offset+512;
#endif
			}
		}else if(isVAR(node->lhs)){		// 添字が単純変数
			if(exist_loc_TDXWK(1,node->str,node->lhs->str)){
				printf("; reuse TDXWK for %s:%s)\n",node->str,node->lhs->str);
				LDX_V("TDXWK");
				return 0;
			}
			gen_save(save);
			LDD_V(node->lhs->str);
			ADD_V(node->str);			// 配列の添字はAccABにある
			TDX();						// TFR D,X
			add_loc_TDXWK_var(1,node->str,node->lhs->str);
			gen_restore(save);
			return 0;
		}else if((node->lhs->kind==ND_ADD
		&&       isNUM(node->lhs->rhs) && node->lhs->rhs->val>=0 && node->lhs->rhs->val<256)){
			// V:expr+offset) の場合、offset計算は外に出せる(ただしoffset<256)
			// (ND_ARRAY1 str=V (+ (expr) (ND_NUM offset)))
			printf("; array type V:expr+offset) optimize\n");
			int offset = node->lhs->rhs->val;
			if(exist_loc_TDXWK(2,node->str,node->lhs->lhs->str)){
				printf("; reuse TDXWK for %s(%s)\n",node->str,node->lhs->lhs->str);
				LDX_V("TDXWK");
				return offset*2;
			}
			gen_save(save);
			gen_expr(node->lhs->lhs);	// offset以外の添字の計算
			ADD_V(node->str);			// 配列の添字はAccABにある
			TDX();						// TFR D,X
			if(isVAR(node->lhs->lhs)){	// 覚えておく
				add_loc_TDXWK_var(1,node->str,node->lhs->lhs->str);
			}
			gen_restore(save);
			return offset;				// offsetは別に返す
		}
		gen_save(save);
		gen_expr(node->lhs);	// 添字の計算をして
		ADD_V(node->str);		// 配列の添字はAccABにある
		TDX();					// TFR D,X
		gen_restore(save);
		return 0;
	}else if(node->kind==ND_ARRAY2){					// 2バイト間接モード
		if(isNUM(node->lhs)){						// オフセットが定数
			int offset = node->lhs->val;
			if(offset>=0 && offset<=127){
				LDX_V(node->str);
				return	offset*2;
			}else if(offset<=-1 && offset>=-3){
				LDX_V(node->str);
				switch(offset){
				case -3: DEX(); DEX();
				case -2: DEX(); DEX();
				case -1: DEX(); DEX();
						return	0;
				default: break;
				}
			}else if(offset>=128 && offset<255){
				LDX_Vp(node->str,256);
				return	offset*2-256;
#ifndef		O_SPEED
			}else if(offset>=256 && offset<511){
				LDX_Vp(node->str,512);
				return	offset*2-512;
#endif
			}else if(offset<0 && offset>=-127){
				LDX_Vp(node->str,-256);
				return	offset*2+256;
#ifndef	O_SPEED
			}else if(offset<-127 && offset>=-255){
				LDX_Vp(node->str,-512);
				return	offset*2+512;
#endif
			}
		}else if(isVAR(node->lhs)){		// 添字が単純変数
			if(exist_loc_TDXWK(2,node->str,node->lhs->str)){
				printf("; reuse TDXWK for %s(%s)\n",node->str,node->lhs->str);
				LDX_V("TDXWK");
				return 0;
			}
			gen_save(save);
			LDD_V(node->lhs->str);
			ASLD();
			ADD_V(node->str);			// 配列の添字はAccABにある
			TDX();						// TFR D,X
			add_loc_TDXWK_var(2,node->str,node->lhs->str);
			gen_restore(save);
			return 0;
		}else if((node->lhs->kind==ND_ADD
		&&       isNUM(node->lhs->rhs) && node->lhs->rhs->val>=0 && node->lhs->rhs->val<128)){
			// V(expr+offset) の場合、offset計算は外に出せる(ただしoffset<128)
			// ex. (ND_ARRAY2 str=V (+ (expr) (ND_NUM offset)))
			printf("; array type V(expr+offset) optimize\n");
			int offset = node->lhs->rhs->val;
			if(exist_loc_TDXWK(2,node->str,node->lhs->lhs->str)){
				printf("; reuse TDXWK for %s(%s)\n",node->str,node->lhs->lhs->str);
				LDX_V("TDXWK");
				return offset*2;
			}
			gen_save(save);
			gen_expr(node->lhs->lhs);	// offset以外の添字の計算
			ASLD();
			ADD_V(node->str);			// 配列の添字はAccABにある
			TDX();						// TFR D,X
			if(isVAR(node->lhs->lhs)){	// 覚えておく
				add_loc_TDXWK_var(2,node->str,node->lhs->lhs->str);
			}
			gen_restore(save);
			return offset*2;			// offsetは別に返す
		}
		gen_save(save);
		gen_expr(node->lhs);
		ASLD();
		ADD_V(node->str);		// 配列の添字はAccABにある
		TDX();					// TFR D,X
		gen_restore(save);
		return 0;
	}
	error("; what's happen? gen_array_address\n");
	return 0;
}

//
//	if cc is true then AccAB=1 else AccAB=0
//
void
gen_compare(Node *node)
{
	// > (ND_VAR I) 10 )
	if(node==NULL){
		printf(";compare node==NULL !\n");
		return;
	}
	//printf("; compare node? ");print_nodes_ln(node);
	if(!isCompare(node)){
		error("not compare");
	}
//	printf("; gen_compare :");print_nodes_ln(node);
	char	*if_false = new_label();
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
//		printf("; gen_compare expr/var >=,< NUM");print_nodes_ln(node);
		if(isVAR(node->lhs)){
			if(exist_loc_var(loc_D,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				TSTA();					// 2cycle,1byte
			}else if(exist_loc_var(loc_X,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				CPX_I(0);				// 3cycle,3byte
			}else{
				TST_V0(node->lhs->str);	// 6cycle,3byte
			}
		}else{
			gen_expr(node->lhs);
			TSTA();
		}
		if(node->kind==ND_LT){			// <0
			Bxx("PL",if_false);
		}else if(node->kind==ND_GE){	// >=0
			Bxx("MI",if_false);
		}
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		purge_loc_B();
		CLRA();
		return;
	}else if(isNUMorVAR(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){	// ==,!= はCPXで
//		printf("; gen_compare expr ==,!= NUM");print_nodes_ln(node);
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
		}else{
			LDX_V(node->lhs->str);
		}
		if(isNUM(node->rhs)){
			if(node->rhs->val!=0){			// LDXはZフラグが立つので0との比較は省略
				CPX_I(node->rhs->val);
			}
		}else{
			CPX_V(node->rhs->str);
		}
		if(node->kind==ND_EQ){
			Bxx("NE",if_false);
		}else{
			Bxx("EQ",if_false);
		}
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		purge_loc_B();
		CLRA();
		return;
	}else if(isARRAY1(node->lhs) && isNUM(node->rhs)	// 左は間接1バイト、右は定数0-255
		&&   node->rhs->val>=0 && node->rhs->val<=255){
		printf("; gen_compare ARRAY1 .rel. NUM");print_nodes_ln(node);
		gen_ARRAY1(node->lhs);							// 結果はAccB
		CMPB_I(node->rhs->val);
		switch(node->kind){
		case ND_EQ: Bxx("NE",if_false);
					break;
		case ND_NE: Bxx("EQ",if_false);
					break;
		case ND_GE:	Bxx("CS",if_false);
					break;
		case ND_GT:	Bxx("LS",if_false);
					break;
		case ND_LE:	Bxx("HI",if_false);
					break;
		case ND_LT: Bxx("CC",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		purge_loc_B();
		CLRA();
		return;
	}else if(isNUMorVAR(node->rhs)){					// 左は式、右は定数か変数
//		printf("; gen_compare expr ==,!= NUM");print_nodes_ln(node);
		gen_expr(node->lhs);							// AccABに結果がある
		// 以下、AccAB(lhs)とNUM(rhs)を比較
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_IVX(node->rhs);
					Bxx("NE",if_false);
					CMPA_IVX(node->rhs);
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_IVX(node->rhs);
					Bxx("NE",if_true);
					CMPA_IVX(node->rhs);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_IVX(node->rhs);
					Bxx("LT",if_false);
					break;
		case ND_GT:	SUB_IVX(node->rhs);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	SUB_IVX(node->rhs);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LT:	SUB_IVX(node->rhs);
					Bxx("GE",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_true);
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		purge_loc_B();
		CLRA();
		return;
	}else{	// 左も右も式
//		printf("; gen_compare expr ?? expr");print_nodes_ln(node);
		gen_expr(node->lhs);
		PSHD();
		gen_expr(node->rhs);
		TSX();
		// 以下、AccAB(rhs)とStackTOP(lhs)を比較
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	//printf("; gen_compare ND_EQ expr?expr\n");
					CMPB_X(1);
					Bxx("NE",if_false);
					CMPA_X(0);
					Bxx("NE",if_false);
					break;
		case ND_NE:	//printf("; gen_compare ND_NE expr?expr\n");
					CMPB_X(1);
					Bxx("NE",if_true);
					CMPA_X(0);
					Bxx("EQ",if_false);
					break;
		case ND_LT:	//printf("; gen_compare ND_LT expr?expr\n");
					SUB_X(0);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	//printf("; gen_compare ND_LE expr?expr\n");
					SUB_X(0);
					Bxx("LT",if_false);
					break;
		case ND_GT:	//printf("; gen_compare ND_GT expr?expr\n");
					// 128>-128 -> -128+(-128) -> ff80+ff80 -> 0x1ff00 N=1,V=1 is true
					SUB_X(0);
					Bxx("GE",if_true);
					BRA(if_false);
					break;
		case ND_GE:	//printf("; gen_compare ND_GE expr?expr\n");
					SUB_X(0);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_false);
					break;
		default:
					error("gen_compare expr?epr ND_cc not known\n");
		}
		LABEL(if_true);
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		purge_loc_B();
		CLRA();
		INS2();
		return;
	}
	error("unknow condition\n");
}

void
gen_branch_if_true(Node *node,char *label)
{
//	printf("; gen_branch_if_true start\n");
	// > (ND_VAR I) 10 )
	if(node==NULL){
//		printf(";branch_if_true node==NULL !\n");
		return;
	}
	//printf("; compare node? ");print_nodes_ln(node);
	if(!isCompare(node)){
		error("not compare");
	}
//	printf("; gen_branch_if_true %s :",label);print_nodes_ln(node);
	char	*if_false = new_label();
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(node->lhs)){
			if(exist_loc_var(loc_D,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				TSTA();
			}else if(exist_loc_var(loc_X,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				CPX_I(0);
			}else{
				TST_V0(node->lhs->str);
			}
		}else{
			gen_expr(node->lhs);
			TSTA();
		}
		if(node->kind==ND_LT){			// lhs<0
			Bxx("PL",if_false);			// lhs>=0 then false
		}else if(node->kind==ND_GE){	// lhs>=0
			Bxx("MI",if_false);			// lhs<0  then false
		}
		JMP(label);						// jump if true
		LABEL(if_false);
		return;
	}else if(isNUMorVAR(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){	// ==,!= はCPXで
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
		}else{
			LDX_V(node->lhs->str);
		}
		if(isNUM(node->rhs)){
			if(node->rhs->val!=0){			// LDXはZフラグが立つので0との比較は省略
				CPX_I(node->rhs->val);
			}
		}else{
			CPX_V(node->rhs->str);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("NE",if_false);
		}else{						// lhs!=rhs
			Bxx("EQ",if_false);	
		}
		LABEL(if_true);
		JMP(label);
		LABEL(if_false);
		return;
	}else if(isARRAY1(node->lhs) && isNUM(node->rhs) && isEQorNE(node)
			&& node->rhs->val>=0 && node->rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_branch_if_true ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X(offset);
		if(node->rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(node->rhs->val);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("NE",if_false);
		}else{						// lhs!=rhs
			Bxx("EQ",if_false);	
		}
		LABEL(if_true);
		JMP(label);
		LABEL(if_false);
	}else if(isARRAY2(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){// 配列との ==,!= はCPXで
//		printf("; gen_branch_if_true ARRAY2 ==/!= NUMorVAR\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDX_X(offset);
		if(isNUM(node->rhs)){
			if(node->rhs->val!=0){			// LDXはZフラグが立つので0との比較は省略
				CPX_I(node->rhs->val);
			}
		}else{
			CPX_V(node->rhs->str);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("NE",if_false);
		}else{						// lhs!=rhs
			Bxx("EQ",if_false);	
		}
		LABEL(if_true);
		JMP(label);
		LABEL(if_false);
		return;
	}else if(isNUMorVAR(node->rhs)){			// 左は式、右は定数か変数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_IVX(node->rhs);
					Bxx("NE",if_false);
					CMPA_IVX(node->rhs);
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_IVX(node->rhs);
					Bxx("NE",if_true);
					CMPA_IVX(node->rhs);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_IVX(node->rhs);
					Bxx("LT",if_false);
					break;
		case ND_GT:	SUB_IVX(node->rhs);
					Bxx("GT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	SUB_IVX(node->rhs);
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LT:	SUB_IVX(node->rhs);
					Bxx("GE",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_true);
		JMP(label);
		LABEL(if_false);
		return;
	}else{
		gen_expr(node->lhs);
		PSHD();
		gen_expr(node->rhs);
		TSX();
		// 以下、AccAB(rhs)とStackTOP(lhs)を比較(rhs-lhsの値を見ている）
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_X(1);
					Bxx("NE",if_false);
					CMPA_X(0);
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_X(1);
					Bxx("NE",if_true);
					CMPA_X(0);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_X(0);
					Bxx("GE",if_false);	// rhs-lhs<0 (lhs<rhs) then true
					break;
		case ND_GT:	SUB_X(0);
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LE:	SUB_X(0);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LT:	SUB_X(0);
					Bxx("LT",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_true);
		INS2();
		JMP(label);
		LABEL(if_false);
		INS2();
		return;
	}
}
void
gen_branch_if_false(Node *node,char *label)
{
//	printf("; gen_branch_if_false start\n");
	// > (ND_VAR I) 10 )
	if(node==NULL){
		printf(";branch_if_false node==NULL !\n");
		return;
	}
	//printf("; compare node? ");print_nodes_ln(node);
	if(!isCompare(node)){
		error("not compare");
	}
//	printf("; gen_branch_if_false %s :",label);print_nodes_ln(node);
	char	*if_false = new_label();
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(node->lhs)){
			if(exist_loc_var(loc_D,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				TSTA();
			}else if(exist_loc_var(loc_X,node->lhs->str)){
				printf("; TST _%s optimized\n",node->lhs->str);
				CPX_I(0);
			}else{
				TST_V0(node->lhs->str);
			}
		}else{
			gen_expr(node->lhs);
			TSTA();
		}
		if(node->kind==ND_LT){			// <0
			Bxx("MI",if_true);
		}else if(node->kind==ND_GE){	// >=0
			Bxx("PL",if_true);
		}
		JMP(label);
		LABEL(if_true);
		return;
	}else if(isNUMorVAR(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){	// ==,!= はCPXで
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
		}else{
			LDX_V(node->lhs->str);
		}
		if(isNUM(node->rhs)){
			if(node->rhs->val!=0){			// LDXはZフラグが立つので0との比較は省略
				CPX_I(node->rhs->val);
			}
		}else{
			CPX_V(node->rhs->str);
		}
		if(node->kind==ND_EQ){
			Bxx("EQ",if_true);
		}else{
			Bxx("NE",if_true);
		}
		LABEL(if_false);
		JMP(label);
		LABEL(if_true);
		return;
	}else if(isARRAY1(node->lhs) && isNUM(node->rhs) && isEQorNE(node)
			&& node->rhs->val>=0 && node->rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_branch_if_false ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X(offset);
		if(node->rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(node->rhs->val);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("EQ",if_true);
		}else{						// lhs!=rhs
			Bxx("NE",if_true);	
		}
		LABEL(if_false);
		JMP(label);
		LABEL(if_true);
	}else if(isARRAY2(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){// 配列との ==,!= はCPXで
//		printf("; gen_branch_if_false ARRAY2 ==/!= NUMorVAR\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDX_X(offset);
		if(isNUM(node->rhs)){
			if(node->rhs->val!=0){			// LDXはZフラグが立つので0との比較は省略
				CPX_I(node->rhs->val);
			}
		}else{
			CPX_V(node->rhs->str);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("EQ",if_true);
		}else{						// lhs!=rhs
			Bxx("NE",if_true);	
		}
		LABEL(if_false);
		JMP(label);
		LABEL(if_true);
		return;
	}else if(isNUMorVAR(node->rhs)){			// 左は式、右は定数か変数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_IVX(node->rhs);
					Bxx("NE",if_false);
					CMPA_IVX(node->rhs);
					Bxx("EQ",if_true);
					break;
		case ND_NE:	CMPB_IVX(node->rhs);
					Bxx("NE",if_true);
					CMPA_IVX(node->rhs);
					Bxx("NE",if_true);
					break;
		case ND_GE:	SUB_IVX(node->rhs);
					Bxx("GE",if_true);
					break;
		case ND_GT:	SUB_IVX(node->rhs);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_true);
					break;
		case ND_LE:	SUB_IVX(node->rhs);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_true);
					break;
		case ND_LT:	SUB_IVX(node->rhs);
					Bxx("LT",if_true);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_false);
		JMP(label);
		LABEL(if_true);
		return;
	}else{
		gen_expr(node->lhs);
		PSHD();
		gen_expr(node->rhs);
		TSX();
		// 以下、AccAB(rhs)とStackTOP(lhs)を比較(rhs-lhsの値を見ている）
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_X(1);
					Bxx("NE",if_false);
					CMPA_X(0);
					Bxx("EQ",if_true);
					break;
		case ND_NE:	CMPB_X(1);
					Bxx("NE",if_true);
					CMPA_X(0);
					Bxx("NE",if_true);
					break;
		case ND_LT:	//print_debug("ND_LT"); // lhs < rhs?   (ie. rhs-lhs>0)
					SUB_X(0);
					Bxx("GE",if_true);	// rhs-lhs<0 (lhs<rhs) then true
					break;
		case ND_LE:	SUB_X(0);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_true);
					break;
		case ND_GT:	SUB_X(0);
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("EQ",if_true);
					break;
		case ND_GE:	SUB_X(0);
					Bxx("LT",if_true);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_false);
		INS2();
		JMP(label);
		LABEL(if_true);
		INS2();
		return;
	}
}

//
//	間接1バイト変数の計算。計算結果はAccBに置く。CLRA無し。
//
void gen_ARRAY1(Node *node)
{
#if	1
	int offset = gen_array_address(node,"");
	LDAB_X(offset);
	return;
#else
	if(isNUM(node->lhs) && node->lhs->val>=0 && node->lhs->val<=255){	// 添字が小さな定数
		LDX_V(node->str);
		int offset = (node->lhs->val);
		LDAB_X(offset);
		return;
	}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-4){
		LDX_V(node->str);
		if(node->lhs->val<0){ DEX(); }
		if(node->lhs->val<-1){ DEX(); }
		if(node->lhs->val<-2){ DEX(); }
		if(node->lhs->val<-3){ DEX(); }
		LDAB_X(0);
		return;
	}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-256){
		LDX_Vp(node->str,-256);
		LDAB_X(256+(node->lhs->val));
		return;
	}
	gen_expr(node->lhs);		// 添字の計算をして
#if	0
	LDX_V(node->str);			// サイズは小さくなるが...
	JSR("ADX");
	LDD_X(0);
#else
	ADD_V(node->str);			// 配列のアドレスはAccABにある
	TDX();						// TFR D,X
	LDAB_X(0);
#endif
	return;
#endif
}

void gen_expr(Node *node)
{
//	printf(";start gen expr: ");print_nodes_ln(node);
	if(node==NULL){
		printf(";gen node==NULL !\n");
		return;
	}
//	printf(";gen expr node=");print_nodes_ln(node);
	switch(node->kind){
	// 変数、配列の右辺値
	case ND_VAR:
//			printf("; gen var\n");
			LDD_V(node->str);
			return;
	case ND_ARRAY1: {
//			printf("; gen array1\n");
//			printf("; ");print_nodes_ln(node->lhs);
			int offset = gen_array_address(node,"");
			LDAB_X(offset);
			CLRA();
			}
			return;
	case ND_ARRAY2: {
			int offset = gen_array_address(node,"");
			LDD_X(offset);
			}
			return;
	// 数値
	case ND_NUM:
			LDD_I(node->val);			// LDD_Iの中でoptimizeしている
			return;
	case ND_PGEND:
			LDD_IV("LPGEND");
			return;
	case ND_INKEY:
			JSR("ASCIN");
			TAB();
			CLRA();
			return;
	case ND_INPUT:
			JSR("INPUT");
			return;
	case ND_TIMER:
			LDD_V("TIMER");
			return;
	case ND_CURSOR:
			LDD_V("CURSOR");
			return;
	case ND_CURSORADRS:
			JSR("CURPOS");
			STX_L("_TMP1");
			LDD_L("_TMP1");
			return;
	case ND_KEYBOARD:{
			char	*label = new_label();
#if			1
			JSR("KBIN_SUB");
#else
			JSR("KBIN");
			Bxx("CS",label);
			TAB();
			SKIP1();
			LABEL(label);
			CLRB();
			purge_loc_B();
			CLRA();
#endif
			}
			return;
	default:
			break;
	}
	// 単項演算子
	switch (node->kind) {
	case ND_ABS:
			gen_expr(node->lhs);
			ABSD();
			return;
	case ND_NEG:
			if(isNUM(node->lhs)){
				printf("; NEG %d => %d\n",node->lhs->val,-(node->lhs->val));
				LDD_I(-(node->lhs->val));
				return;
			}
			gen_expr(node->lhs);
			NEGD();
			return;
	case ND_ASLD:
			gen_expr(node->lhs);
			for(int val=node->val;val>0;val--){
				ASLD();
			}
			return;
	case ND_RAND:
			gen_expr(node->lhs);
			JSR("RANDOM");
			return;
	case ND_MOD:
			if(isNUM(node->lhs)){		// 定数の場合はMODを拾ってくるだけで良い
				LDD_V("MOD");
				return;
			}
			gen_expr(node->lhs);
			LDD_V("MOD");
			return;
	case ND_CALLVAL:
			LDD_V("CALLVAL");
			return;
	case ND_ANDI:
			gen_expr(node->lhs);
			AND_I(node->val);
			return;
	case ND_ANDI_MOD:						// MODを更新するANDI
			gen_expr(node->lhs);
			AND_I(node->val);
			STD_V("MOD");
			return;
	default:
		break;
	}
	// 以下は二項演算
	switch (node->kind) {
	case ND_ADD:
			if(isVAR(node->lhs)&&isVAR(node->rhs)		// 単純変数同士の加算
			&& exist_loc_var(loc_D,node->rhs->str)){	// さっき使ったばかりの変数か?
				Node *old = node;						// 左右を入れ替える
				node = new_copy_node(old);
				node->lhs = old->rhs;
				node->rhs = old->lhs;
			}
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				ADD_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				ADD_V(node->rhs->str);
			}else if(isARRAY(node->rhs)){
				int offset = gen_array_address(node->rhs,"D");
				ADD_X(offset);
					return;
			}else{
				PSHD();
				gen_expr(node->rhs);
				TSX();
				ADD_X(0);
				INS2();
				return;
			}
			break;
	case ND_SUB:
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				SUB_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				SUB_V(node->rhs->str);
			}else{
				PSHD();
				gen_expr(node->rhs);
				NEGD();
				TSX();
				ADD_X(0);
				INS2();
				return;
			}
			break;
	case ND_MUL:
			if(isVAR(node->lhs)&&isVAR(node->rhs)		// 単純変数同士の乗算
			&& exist_loc_var(loc_D,node->rhs->str)){	// さっき使ったばかりの変数か?
				Node *old = node;						// 左右を入れ替える
				node = new_copy_node(old);
				node->lhs = old->rhs;
				node->rhs = old->lhs;
			}
			if(isCompare(node->lhs)){	// 比較演算結果との乗算 ex. (x>=y)*32
				char *label = new_label();
				gen_expr(node->lhs);
				PSHD();
				gen_expr(node->rhs);
				TSX();
				TST_X(1);				// 1 or 0
				Bxx("NE",label);		// if 1, rhs is result.
				CLRD();					// otherwise 0
				LABEL(label);
				INS2();
				purge_loc_D();
				return;
			}
			if(isNUM(node->rhs)){
				int	shift=0;
				switch(node->rhs->val){
				case 256:
					gen_expr(node->lhs);
					MUL256();
					return;
				case 20:
				case 10:
				case 5:
					gen_expr(node->lhs);
					STD_L("_TMP1");
					ASLD_N(2);
					ADD_L("_TMP1");
					if(node->rhs->val==20){
						ASLD();
						ASLD();
					}else if(node->rhs->val==10){
						ASLD();
					}
					return;
				case 12:
				case 6:
				case 3:
					gen_expr(node->lhs);
					STD_L("_TMP1");
					ASLD_N(1);
					ADD_L("_TMP1");
					if(node->rhs->val==12){
						ASLD();
						ASLD();
					}else if(node->rhs->val==6){
						ASLD();
					}
					return;
				case 18:
				case 9:
					gen_expr(node->lhs);
					STD_L("_TMP1");
					ASLD_N(3);
					ADD_L("_TMP1");
					if(node->rhs->val==18){
						ASLD();
					}
					return;
				case 14:
				case 7:
					gen_expr(node->lhs);
					STD_L("_TMP1");
					ASLD_N(3);
					SUB_L("_TMP1");
					if(node->rhs->val==14){
						ASLD();
					}
					return;
				case 17:
				case 15:
					gen_expr(node->lhs);
					STD_L("_TMP1");
					ASLD_N(4);
					if(node->rhs->val==15){
						SUB_L("_TMP1");
					}else{
						ADD_L("_TMP1");
					}
					return;
				case 128:	shift++;
				case 64:	shift++;
				case 32:	shift++;
				case 16:	shift++;
				case 8:		shift++;
				case 4:		shift++;
				case 2:		shift++;
					gen_expr(node->lhs);
					ASLD_N(shift);
					return;
				default:;
				}
			}
			gen_expr(node->lhs);
			PSHD();
			gen_expr(node->rhs);
			TSX();
			JSR("MULTIPLY");
			INS2();
			break;
	case ND_DIV:
			// 余り(MOD)の処理があるので簡単ではない
			// 符号拡張に注意
			if(isNUM(node->lhs) && isNUM(node->rhs)){	// 定数同士の除算の場合、大抵はMODの設定
				int16_t	x=node->lhs->val;
				int16_t	y=node->rhs->val;
				int16_t	d=x/y;
				int16_t m=abs(x%y);						// GAME68では余りは常に正
				LDX_I(m);
				STX_V("MOD");
				LDD_I(d);
				return;
			}
			if(isSameVAR(node->lhs,node->rhs)){			// 同じ変数の割り算 K/K など
				LDX_I(0);
				STX_V("MOD");
				LDD_I(1);
				return;
			}
			if(isNUM(node->rhs) && node->rhs->val==2){	// 2で割る時は特殊処理
				printf("; DIV /2 debug: ");print_nodes_ln(node);
				if(isVAR(node->lhs)){		// 単純変数/2
					LDAB_V(node->lhs->str);	// 4 3
					ANDB_I(1);				// 2 2
					STAB_V("MOD");			// 5 3
					CLR_V0("MOD");			// 6 3	↑16 11
					gen_expr(node->lhs);
				}else{						// expr/2
					gen_expr(node->lhs);
					PSHB();					// 4 1
					ANDB_I(1);				// 2 2
					STAB_V("MOD");			// 5 3
					CLR_V0("MOD");			// 6 3
					PULB();					// 4 1	↑21 10
				}
				char	*label2 = new_label();
									// If the number is negative, add +1
				printf("\tASRA\n");	// 2 1 duplicate N(bit7 to 6), shift bit6-1 bit0->C
				printf("\tROLA\n");	// 2 1 restore bit7-0, bit7(N) to C
				printf("\tADCB\t#0\n");// 2 2 add carry to D
				printf("\tADCA\t#0\n");// 2 2
				ASRD_N(1);			// 2+2 1+1 div by 2
				return;
			}
			// 変数/定数
			if(isVAR(node->lhs) && isNUM(node->rhs) && node->rhs->val>0){
//				printf("; DIV debug: ");print_nodes_ln(node);
				int	val=node->rhs->val;
				char *str=node->lhs->str;
				int	shift=0;
				int	mask=val-1;
				switch(val){
				case	16384:shift++;
				case	8192:shift++;
				case	4096:shift++;
				case	2048:shift++;
				case	1024:shift++;
				case	512:shift++;
				case	256:shift++;
				case	128:shift++;
				case	64:	shift++;
				case	32:	shift++;
				case	16:	shift++;
				case	8:	shift++;
				case	4:	shift++;
				case	2:	shift++;
							LDAB_I(shift);
							PSHB();
							LDD_I(mask);
							PSHD();
							LDD_V(str);
							JSR("DIVPOW2");
							INS2();
							INS();
							return;
				case	1:	// 要る?
							CLR_V("MOD");
							LDD_V(str);
							return;
				default: break;
				}
			}
			gen_expr(node->lhs);
			PSHD();
			gen_expr(node->rhs);
			PSHD();
			JSR("DIVIDE");
			INS2();
			INS2();
			break;
	case ND_ABS: {
			char	*LABS = new_label();
			gen_expr(node->lhs);
			TSTA();
			Bxx("PL",LABS);
			NEGD();
			LABEL(LABS);
			}
			break;
	case ND_EQ: 
	case ND_NE:
	case ND_GT:
	case ND_GE:
	case ND_LT:
	case ND_LE:{
			printf("; ");print_nodes_ln(node);
			char	*if_false = new_label();
			gen_compare(node);
			}
			break;
	case ND_BITAND:
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				AND_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				AND_V(node->rhs->str);
			}else{
				PSHD();
				gen_expr(node->rhs);
				TSX();
				AND_X(0);
				INS2();
				return;
			}
			break;
	case ND_BITOR:
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				OR_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				OR_V(node->rhs->str);
			}else{
				PSHD();
				gen_expr(node->rhs);
				TSX();
				OR_X(0);
				INS2();
				return;
			}
			break;
	case ND_BITXOR:
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				EOR_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				EOR_V(node->rhs->str);
			}else{
				PSHD();
				gen_expr(node->rhs);
				TSX();
				EOR_X(0);
				INS2();
				return;
			}
			break;
	default:
			error("Unknown token\n");
			break;
	}
}


void
gen_stmt(Node *node)
{
	static	Node	*current = NULL;

	//printf("gen_stmt start\n");fflush(stdout);
	if(node==NULL){
		printf(";gen_stmt node==NULL !\n");
		return;
	}
	//printf("gen_stmt node->kind=%d\n",node->kind);fflush(stdout);
	printf("; ");print_nodes_ln(node);
	switch(node->kind){
	case ND_NOP:	// no operation
		return;
	case ND_LINENUM:
//		printf("*\t");print_line(node->str);printf("\n");
		// IF/GOTO/GOSUBの飛び先として使われていない行番号はラベルにしない
		if(usedLINENO(node->val)){
			LABEL(new_line_label(node->val));
			purge_loc_all();	// レジスタ割り当てクリア
		}
		current = node;
		return;
	case ND_ASM: {
			char *p=node->str;
			if(*p==' '){
				printf("\t");
				while(*p==' '){
					p++;
				}
			}
			printf("%s\n",p);
			purge_loc_all();	// レジスタ割り当てクリア
		}
		return;
	case ND_ASSIGN: {
			Node *lhs = node->lhs;		// 代入先変数
			Node *rhs = node->rhs;		// 右辺
			if(node->lhs->kind==ND_VAR){
				gen_expr(rhs);
				gen_store_var(lhs);
			}else if(isADDorSUB(rhs) &&	isNUMorVAR(rhs->rhs) && isSameARRAY(lhs,rhs->lhs)){
				//  A(X)=A(X)+n
				printf("; folding ARRAY self assign: ");print_nodes_ln(node);
				int offset = gen_array_address(lhs,"");
				if(isARRAY1(lhs)){
					LDAB_X(offset);
					CLRA();
				}else{
					LDD_X(offset);
				}
				if(isNUM(rhs->rhs)){
					if(rhs->kind==ND_ADD){
						ADD_I(rhs->rhs->val);
					}else{
						SUB_I(rhs->rhs->val);
					}
				}else{
					if(rhs->kind==ND_ADD){
						ADD_V(rhs->rhs->str);
					}else{
						SUB_V(rhs->rhs->str);
					}
				}
				if(isARRAY1(lhs)){
					STAB_X(offset);
				}else{
					STD_X(offset);
				}
			}else if(lhs->kind==ND_ARRAY1){
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					CLR1_X(offset);
					return;
				}else if(isNUMorVAR(rhs)){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					LDAB_IVX(rhs);
					STAB_X(offset);
					return;
				}
#if	1
				if(!has_side_effect(lhs)){
					gen_expr(rhs);
					int offset = gen_array_address(lhs,"B");
					STAB_X(offset);
					return;
				}
#endif
				gen_expr(lhs->lhs);			// calculate subscript
				ADD_V(lhs->str);				// 左辺のアドレスはDにある
				PSHD();
				gen_expr(rhs);
				TSX();
				LDX_X(0);
				INS2();
				STAB_X(0);
				return;
			}else if(lhs->kind==ND_ARRAY2){
				//  (ND_ASSIGN (ND_ARRAY2 str=N (ND_VAR str=I)) (ND_NUM 0))
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					CLR_X(offset);
					return;
				}else if(isNUMorVAR(rhs)){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					LDD_IVX(rhs);
					STD_X(offset);
					return;
				}
#if	1
				if(!has_side_effect(lhs)){
					gen_expr(rhs);
					int offset = gen_array_address(lhs,"D");
					STD_X(offset);
					return;
				}
#endif
				gen_expr(lhs->lhs);			// calculate subscript
				ASLD();
				ADD_V(lhs->str);				// 左辺のアドレスはDにある
				PSHD();
				gen_expr(rhs);
				TSX();
				LDX_X(0);
				INS2();
				STD_X(0);
				return;
			}else{
				error("ASSIGN VAR error not var/array\n");
			}
		}
		return;
	case ND_IF: {
//			printf("; ");print_nodes(node);printf("\n");
			char	*IF_TRUE	= new_label();
			char	*IF_FALSE	= new_label();
			char	*NEXT_LINE	= new_line_label(current->rhs->val);
			Node	*lhs = node->lhs;
			if(isVAR(lhs)){							// if 変数 の場合
				LDX_V(lhs->str);
				Bxx("NE",IF_TRUE);
				JMP(NEXT_LINE);
				LABEL(IF_TRUE);
				return;
#if	0
			}else if((lhs->kind==ND_EQ) && (lhs->lhs->kind==ND_KEYBOARD) && isNUM(lhs->rhs)){
				if(lhs->rhs->val==0){	// check no keyin
					char *nokey = new_label();
					JSR("KBIN");		// AccA = keyin
					Bxx("CS",IF_TRUE);
					JMP(NEXT_LINE);
					LABEL(IF_TRUE);
				}else{
					// (== (ND_KEYBOARD) (ND_NUM 82))
					char *nokey = new_label();
					JSR("KBIN");		// AccA = keyin
					Bxx("CS",IF_FALSE);
					CMPB_I(lhs->rhs->val);
					Bxx("EQ",IF_TRUE);
					LABEL(IF_FALSE);
					JMP(NEXT_LINE);
					LABEL(IF_TRUE);
				}
				return;
#endif
			}else if(isCompare(lhs)){
//				printf("; call gen_branch_if_false\n");
				gen_branch_if_false(lhs,NEXT_LINE);
				return;
			}
			gen_expr(lhs);
			ABA();
			Bxx("NE",IF_TRUE);
			Bxx("CS",IF_TRUE);
			JMP(NEXT_LINE);
			LABEL(IF_TRUE);
		}
		return;
	case ND_IFGOTO: {
//			printf("; ");print_nodes(node);printf("\n");
			char	*IF_TRUE	= new_label();
			char	*IF_FALSE	= new_label();
			char	*GOTOLINE	= new_line_label(node->val);
			if(isVAR(node->lhs)){							// if 変数 の場合
				LDX_V(node->lhs->str);
				Bxx("EQ",IF_FALSE);
				JMP(GOTOLINE);
				LABEL(IF_FALSE);
				return;
			}else if(isCompare(node->lhs)){					// if 条件の場合
//				printf("; call gen_branch_if_true\n");
				gen_branch_if_true(node->lhs,GOTOLINE);
				return;
			}
			gen_expr(node->lhs);							// if 式 の演算結果はAccABに入る’
			TSTB();
			Bxx("NE",IF_TRUE);
			TSTA();
			Bxx("EQ",IF_FALSE);
			LABEL(IF_TRUE);
			JMP(GOTOLINE);
			LABEL(IF_FALSE);
		}
		return;
	case ND_RETURN:
		printf("\tRTS\n");
		purge_loc_all();	// レジスタ割り当てクリア
		return;
	case ND_GOTO:
		if(node->lhs->kind!=ND_NUM){
			error("GOTO linenumber not constant\t");
			return;
		}
		JMP(new_line_label(node->val));
		purge_loc_all();	// レジスタ割り当てクリア
		return;
	case ND_GOSUB:
		if(node->lhs->kind!=ND_NUM){
			error("GOSUB linenumber not constant\t");
			return;
		}
		JSR(new_line_label(node->val));
		purge_loc_all();	// レジスタ割り当てクリア
		return;
	case ND_DO:
		LABEL(new_do_label(node->val));
		purge_loc_all();	// レジスタ割り当てクリア
		return;
	case ND_UNTIL: {
			char	*DO_LOOP = new_do_label(node->val);
			char	*DO_SKIP = new_label();
			if(isCompare(node->lhs)){
//				printf("; until compre node: ");print_nodes_ln(node->lhs);
				gen_branch_if_false(node->lhs,DO_LOOP);
				return;
			}
			printf("; until expr: ");print_nodes_ln(node->lhs);
			gen_expr(node->lhs);
			ABA();
			Bxx("NE",DO_SKIP);
			Bxx("VS",DO_SKIP);
			JMP(DO_LOOP);
			LABEL(DO_SKIP);
		}
		return;
	case ND_FOR: {
			// (ND_FOR J (ND_SETVAR J 0 ) 10 )
			// 終値は+1しておく（比較がGEで行える）
			char	*FOR_LABEL=new_for_label(node->val);
			char	*TO_LABEL=new_to_label(node->val);
			FORTO[node->val].type  = node->rhs->kind;
			FORTO[node->val].var   = node->str;
			FORTO[node->val].loop  = FOR_LABEL;
			FORTO[node->val].to    = TO_LABEL;
			gen_stmt(node->lhs);
			if(isNUM(node->rhs)){	// 終値が定数の時
				FORTO[node->val].val = node->rhs->val+1;
//				LDX_I(node->rhs->val+1);
//				STX_L(TO_LABEL);
			}else if(isVAR(node->rhs)){	// 終値が定数のときは、作業用領域に格納しておく
				LDX_V(node->rhs->str);
				INX();
				STX_L(TO_LABEL);
			}else{
				gen_expr(node->rhs);
				ADD_I(1);
				STD_L(TO_LABEL);
			}
			LABEL(FOR_LABEL);	// NEXTから戻ってくる場所
			purge_loc_all();	// レジスタ割り当てクリア
		}
		return;
	case ND_NEXT: {
		// (ND_NEXT J (+ (ND_VAR J) 1 ))
		char	*FOR_LABEL=FORTO[node->val].loop;
		char	*TO_LABEL =FORTO[node->val].to;
		char	*NEXT_LABEL=new_label();
		Node	*node_step = new_unary(ND_SETVAR,node->lhs);
		node_step->str = node->str;
		Node	*opt = node_opt(node_step);
//		printf(";  ");print_nodes_ln(node_step);
//		printf(";=>");print_nodes_ln(opt);
		if(opt->kind==ND_INCVAR
		&& isSameVAR(opt,node)==0){	// (ND_INCVAR str=I)
			LDX_V(opt->str);
			INX();
			STX_V(opt->str);
		}else if(opt->kind==ND_INC2VAR
		&& isSameVAR(opt,node)==0){	// (ND_INC2VAR str=I)
			LDX_V(opt->str);
			INX();
			INX();
			STX_V(opt->str);
		}else{
			gen_stmt(opt);
		}
		LDD_V(node->str);
		if(FORTO[node->val].type==ND_NUM){
			SUB_I(FORTO[node->val].val);
		}else{
			SUB_L(TO_LABEL);
		}
		Bxx("PL",NEXT_LABEL);
//		LABEL(to_FOR);
		JMP(FOR_LABEL);
		LABEL(NEXT_LABEL);
		}
		return;
	case ND_PRINTCR:
		JSR("PRINTCR");
		return;
	case ND_PRINTSTR:{
		char	*label = new_label();
		printf("; %s='%s'\n",label,node->str);fflush(stdout);
		LDX_IL(label);
		JSR("PRINTSTR");
		STRING_FCC[string_count].label = label;
		STRING_FCC[string_count].str   = escape_bmchar(node->str);
		STRING_FCC[string_count].orig  = node->str;
		string_count++;
		}
		return;
	case ND_PRINT:
		gen_expr(node->lhs);
		JSR("PRINTL");
		return;
	case ND_PRINTR:
		gen_expr(node->rhs);
		PSHD();
		gen_expr(node->lhs);
		JSR("PRINTR");
		INS2();
		return;
	case ND_PRINTTAB:
		gen_expr(node->lhs);
		JSR("PRINTTAB");
		return;
	case ND_PRINTCH:
		if(isNUM(node->lhs)){
			LDAA_I(node->lhs->val);
		}else if(isVAR(node->lhs)){
			LDAA_V1(node->lhs->str);
		}else{	
			gen_expr(node->lhs);
			printf("\tTBA\n");
		}
		JSR("ASCOUT");
		return;
	case ND_PRINTHEX4:
		gen_expr(node->lhs);
		JSR("PRHEX4");
		return;
	case ND_PRINTHEX2:
		gen_expr(node->lhs);
		JSR("PRHEX2");
		return;
	case ND_SETVAR: {
			if(isNUM(node->lhs)){
				if(exist_loc_const(loc_X,node->lhs->val)){
					printf("; LDX_I #%d optimized\n",node->lhs->val);
					STX_V(node->str);
				}else if(exist_loc_const(loc_D,node->lhs->val)){
					printf("; LDD_I #%d optimized\n",node->lhs->val);
					STD_V(node->str);
				}else{
					LDX_I(node->lhs->val);
					STX_V(node->str);
				}
			}else if(isVAR(node->lhs)){
				if(exist_loc_var(loc_X,node->lhs->str)){
					printf("; LDX_V var %s optimized\n",node->lhs->str);
					STX_V(node->str);
				}else if(exist_loc_var(loc_D,node->lhs->str)){
					printf("; LDD_V var %s optimized\n",node->lhs->str);
					STD_V(node->str);
				}else{
					LDX_V(node->lhs->str);
					STX_V(node->str);
				}
			}else if(isARRAY(node->lhs)){			// Z=A(I) or Z=A:I)
				printf("; ND_SETVAR ");print_nodes_ln(node);
				int offset = gen_array_address(node->lhs,"");	// IXにA(I) or A:I)のアドレスが入る
				if(node->lhs->kind==ND_ARRAY1){
					LDAB_X(offset);
					STAB_V(node->str);
					CLR_V0(node->str);
				}else if(node->lhs->kind==ND_ARRAY2){
					LDX_X(offset);
					STX_V(node->str);
				}else{
					printf("; ND_SETVAR error ");print_nodes_ln(node);
					error("; what happen?");
				}
			}else if((node->lhs->kind==ND_NEG)		// Z=-Z  (ND_SETVAR Z (ND_NEG (ND_VAR Z)))
				  && (isVAR(node->lhs->lhs))
				  && (strcmp(node->str,node->lhs->lhs->str)==0)){
					NEG_V(node->str);
			}else{
				gen_expr(node->lhs);
				STD_V(node->str);
			}
		}
		return;
	case ND_SETVAR_N:
		if(isNUMorVAR(node->rhs) && node->lhs->kind==ND_CELL){	// 定数/変数の連続代入
			if(isNUM(node->rhs)){
				LDX_I(node->rhs->val);
			}else{
				LDX_V(node->rhs->str);
			}
			Node	*p = node->lhs;
			while(p->kind==ND_CELL){
				if(!isVAR(p->lhs)){
					error("; ND_SETVAR error. not Variable\n");
				}
				STX_V(p->lhs->str);
				p = p->rhs;
			}
			return;
		}
		error("; unknown ND_SETVAR pattern\n");
		break;
	case ND_SETARY1_N: {
		// ; (ND_SETARY1_N (ND_ARRAY1 str=B (ND_NUM 0)) (((ND_NUM 0) (ND_NUM 138)) ((ND_NUM 1) (ND_NUM 148))((ND_NUM 2) (ND_NUM 136))))
			Node	*p = node->rhs;
			LDX_V(node->lhs->str);
			Node	*prev=new_node_none();
			while(p->kind==ND_CELL){
				Node	*lhs = p->lhs;
//				printf("; SETARY1_N ");print_nodes(prev);printf(" ");print_nodes_ln(p);
				if(!isNUM(lhs->lhs) && !isNUMorVAR(lhs->rhs)){
					error("; illegal ND_SETARY1 parameter\n");
				}
				if(isNUM(lhs->rhs)){
					if(!isNUM(prev) || (prev->val!=lhs->rhs->val)){
						LDAB_I(lhs->rhs->val);
					}
				}else{
					if(!isVAR(prev) || strcmp(prev->str,lhs->rhs->str)!=0){
						LDAB_V(lhs->rhs->str);
					}
				}
				STAB_X(lhs->lhs->val);
				prev = p->lhs->rhs;
				p = p->rhs;
			}
			return;
		}
		error("; unknown ND_SETARY1 pattern\n");
		break;
	case ND_SETARY2_N: {
		// ; (ND_SETARY2_N (ND_ARRAY1 str=B (ND_NUM 0)) (((ND_NUM 0) (ND_NUM 138)) ((ND_NUM 1) (ND_NUM 148))((ND_NUM 2) (ND_NUM 136))))
			Node	*p = node->rhs;
			LDX_V(node->lhs->str);
			Node	*prev=new_node_none();
			while(p->kind==ND_CELL){
				Node	*lhs = p->lhs;
				printf("; SETARY2_N ");print_nodes(prev);printf(" ");print_nodes_ln(p);
				if(!isNUM(lhs->lhs) && !isNUMorVAR(lhs->rhs)){
					error("; illegal ND_SETARY2 parameter\n");
				}
				if(isNUM(lhs->rhs)){
					if(!isNUM(prev) || (prev->val!=lhs->rhs->val)){
						LDD_I(lhs->rhs->val);
					}
				}else{
					if(!isVAR(prev) || strcmp(prev->str,lhs->rhs->str)!=0){
						LDD_V(lhs->rhs->str);
					}
				}
				STD_X(lhs->lhs->val*2);
				prev = p->lhs->rhs;
				p = p->rhs;
			}
			return;
		}
		error("; unknown ND_SETARY2 pattern\n");
		break;
	case ND_SETTIMER:
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
			STX_V("TIMER");
		}else if(isVAR(node->lhs)){
			LDX_V(node->lhs->str);
			STX_V("TIMER");
		}else{
			gen_expr(node->lhs);
			STD_V("TIMER");
		}
		return;
	case ND_SETCURSOR:
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
			STX_V("CURSOR");
		}else if(isVAR(node->lhs)){
			LDX_V(node->lhs->str);
			STX_V("CURSOR");
		}else{
			gen_expr(node->lhs);
			STD_V("CURSOR");
		}
		return;
	case ND_CALL: {
			char	*buf=calloc(1,256);
			if(isNUM(node->lhs)){
				sprintf(buf,"$%04X",node->lhs->val);
				JSR(buf);
			}else if(isVAR(node->lhs)){
				LDX_V(node->lhs->str);
				JSR_X(0);
			}else{
				gen_expr(node->lhs);
				TDX();
				JSR_X(0);
			}
			STD_V("CALLRET");
			purge_loc_all();
		}
		return;
	case ND_MUSIC:{
		char	*label = new_label();
		printf("; %s='%s'\n",label,node->str);fflush(stdout);
		LDX_IL(label);
		JSR("MUSIC");
		printf("\tCLI\n");
		STRING_FCC[string_count].label = label;
		STRING_FCC[string_count].str   = escape_bmchar(node->str);
		STRING_FCC[string_count].orig  = node->str;
		string_count++;
		}
		return;
	case ND_NEGVAR:
//			printf("; ND_NEGVAR %s\n",node->str);
			NEG_V(node->str);
			return;
	case ND_INCVAR:
//			printf("; ND_INCVAR %s\n",node->str);
			LDX_V(node->str);
			INX();
			STX_V(node->str);
			return;
	case ND_DECVAR:
//			printf("; ND_DEXVAR %s\n",node->str);
			LDX_V(node->str);
			DEX();
			STX_V(node->str);
			return;
	case ND_INC2VAR:
//			printf("; ND_INC2VAR %s\n",node->str);
			LDX_V(node->str);
			INX();
			INX();
			STX_V(node->str);
			return;
	case ND_DEC2VAR:
//			printf("; ND_DEX2VAR %s\n",node->str);
			LDX_V(node->str);
			DEX();
			DEX();
			STX_V(node->str);
			return;
	default:
		//printf(";code1 default\n");
		return;
	}
}


void
prologue()
{
	string_count = 0;
	printf("*\n");
	printf("* generated by GAME-CC compiler\n");
	printf("*\n");
	printf("\tORG\t$2000\n");
	printf("main\tEQU\t*\n");
	LABEL(new_line_label(0));
	printf("\tSTS\t_RETSP\n");
	JSR("INIT_MUSIC");
	LABEL(new_line_label(1));
}

void
epilogue()
{
	LABEL(new_line_label(-1));
	printf("\tLDS\t_RETSP\n");
	printf("\tRTS\n");
	RMB("_RETSP",2);
	RMB("_CPXWK",2);
	RMB("_TMP1",2);
	printf("VAR\tEQU\t*\n");
	RMB("_A",2);
	RMB("_B",2);
	RMB("_C",2);
	RMB("_D",2);
	RMB("_E",2);
	RMB("_F",2);
	RMB("_G",2);
	RMB("_H",2);
	RMB("_I",2);
	RMB("_J",2);
	RMB("_K",2);
	RMB("_L",2);
	RMB("_M",2);
	RMB("_N",2);
	RMB("_O",2);
	RMB("_P",2);
	RMB("_Q",2);
	RMB("_R",2);
	RMB("_S",2);
	RMB("_T",2);
	RMB("_U",2);
	RMB("_V",2);
	RMB("_W",2);
	RMB("_X",2);
	RMB("_Y",2);
	RMB("_Z",2);
	RMB("_CALLRET",2);
	printf("_TIMER\tEQU\t$000A\n");
	printf("_CURSOR\tEQU\t$000F\n");
	for(int i=1; i<=for_count; i++){
		if(FORTO[i].to){
			char	comment[256];
			sprintf(comment,"%s: FOR %s",FORTO[i].loop,FORTO[i].var);
			RMB_comment(FORTO[i].to,2,comment);
		}
	}
	for(int i=0; i<string_count; i++){
		LABEL(STRING_FCC[i].label);
		printf("*\t%s\n",STRING_FCC[i].orig);
		printf("%s",STRING_FCC[i].str);
		printf("\tFCB\t0\n");
	}
	printf("\tINCLUDE\tgamecc.asm\n");
	printf("_LPGEND\tEQU\t*\n");
	printf("\tEND\tmain\n");
	printf("* vim" ":set" " ts=8:\n");
}
// vim:set ts=4:
