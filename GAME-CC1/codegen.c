#include	"common.h"

int	for_to_count;
int	string_count;

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
	char	*label = calloc(1,10);

	sprintf(label,".LF%d",v);
	return	label;
}
char	*
new_to_label(char *str)
{
	char	*label = calloc(1,10);

	sprintf(label,".LT_%s",str);
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
void	INCA()
{
		printf("\tINCA\n");
}
void	INCB()
{
		printf("\tINCB\n");
}
void	DECA()
{
		printf("\tDECA\n");
}
void	DECB()
{
		printf("\tDECB\n");
}
void	TST_V0(char	*str)
{
		printf("\tTST\t_%s\n",str);
}
void	TSX()
{
		printf("\tTSX\n");
}
void	INX()
{
		printf("\tINX\n");
}
void	INX2()
{
		INX();
		INX();
}
void	DEX()
{
		printf("\tDEX\n");
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
void	PULD()
{
		printf("\tPULA\n");
		printf("\tPULB\n");
}

void	ASLD()
{
		printf("\tASLB\n");
		printf("\tROLA\n");
}
void	ASLD_N(int n)
{
		while(n-->0){
			printf("\tASLB\n");
			printf("\tROLA\n");
		}
}
void	ASRD_N(int n)
{
		while(n-->0){
			printf("\tASRA\n");
			printf("\tRORB\n");
		}
}

void	NEGD()
{
		printf("\tNEGA\n");
		printf("\tNEGB\n");
		printf("\tSBCA\t#0\n");
}

void	NEG_V(char *v)
{
		char	*label = new_label();
		printf("\tNEG\t_%s+1\n",v);
		printf("\tBNE\t%s\n",label);
		printf("\tDEC\t_%s\n",v);
		printf("%s\tCOM\t_%s\n",label,v);
}

void Bxx(char *cc, char *label);

void	ABSD()
{
		char	*positive  = new_label();
		TSTA();
		Bxx("PL",positive);
		NEGD();
		LABEL(positive);
}

void	LDD_I(int v)
{
		if(low(v)==0){
			printf("\tCLRB\n");
		}else{
			printf("\tLDAB\t#%d\n", low(v));
		}
		if(high(v)==0){
			printf("\tCLRA\n");
		}else{
			printf("\tLDAA\t#%d\n", high(v));
		}
}
void	LDD_IV(char *v)
{
		printf("\tLDAB\t#_%s\n", 	v);
		printf("\tLDAA\t#_%s/256\n",v);
}
void	LDD_X(int v)
{
		printf("\tLDAB\t%d,X\n",v+1);
		printf("\tLDAA\t%d,X\n",v);
}
void	LDD_V(char *v)
{
		printf("\tLDAB\t_%s+1\n", v);
		printf("\tLDAA\t_%s\n", v);
}
void	LDD_L(char *str)
{
		printf("\tLDAB\t%s+1\n", str);
		printf("\tLDAA\t%s\n", str);
}
void	ADD_I(int v)
{
		printf("\tADDB\t#%d\n", low(v));
		printf("\tADCA\t#%d\n", high(v));
}
void	ADD_IV(char *v)
{
		printf("\tADDB\t#_%s+1\n",	v);
		printf("\tADCA\t#_%s\n",	v);
}
void	ADD_V(char *v)
{
		printf("\tADDB\t_%s+1\n",	v);
		printf("\tADCA\t_%s\n",		v);
}
void	ADD_L(char *str)
{
		printf("\tADDB\t%s+1\n", str);
		printf("\tADCA\t%s\n", str);
}
void	ADD_X(int v)
{
		printf("\tADDB\t%d,X\n", v+1);
		printf("\tADCA\t%d,X\n", v);
}
void	SUB_I(int v)
{
		printf("\tSUBB\t#%d\n", low(v));
		printf("\tSBCA\t#%d\n", high(v));
}
void	SUB_X(int v)
{
		printf("\tSUBB\t%d,X\n", v+1);
		printf("\tSBCA\t%d,X\n", v);
}
void	SUB_V(char *v)
{
		printf("\tSUBB\t_%s+1\n",	v);
		printf("\tSBCA\t_%s\n",		v);
}
void	SUB_L(char *v)
{
		printf("\tSUBB\t%s+1\n",	v);
		printf("\tSBCA\t%s\n",		v);
}
//
void	AND_I(int v)
{
		printf("\tANDB\t#%d\n",low(v));
		printf("\tANDA\t#%d\n",high(v));
}
void	AND_X(int v)
{
		printf("\tANDB\t%d,X\n", v+1);
		printf("\tANDA\t%d,X\n", v);
}
void	AND_V(char *v)
{
		printf("\tANDB\t_%s+1\n",	v);
		printf("\tANDA\t_%s\n",		v);
}
void	AND_L(char *v)
{
		printf("\tANDB\t%s+1\n",	v);
		printf("\tANDA\t%s\n",		v);
}
//
void	OR_I(int v)
{
		printf("\tORAB\t#%d\n", low(v));
		printf("\tORAA\t#%d\n", high(v));
}
void	OR_X(int v)
{
		printf("\tORAB\t%d,X\n", v+1);
		printf("\tORAA\t%d,X\n", v);
}
void	OR_V(char *v)
{
		printf("\tORAB\t_%s+1\n",	v);
		printf("\tORAA\t_%s\n",		v);
}
void	OR_L(char *v)
{
		printf("\tORAB\t%s+1\n",	v);
		printf("\tORAA\t%s\n",		v);
}
//
void	EOR_I(int v)
{
		printf("\tEORB\t#%d\n", low(v));
		printf("\tEORA\t#%d\n", high(v));
}
void	EOR_X(int v)
{
		printf("\tEORB\t%d,X\n", v+1);
		printf("\tEORA\t%d,X\n", v);
}
void	EOR_V(char *v)
{
		printf("\tEORB\t_%s+1\n",	v);
		printf("\tEORA\t_%s\n",		v);
}
void	EOR_L(char *v)
{
		printf("\tEORB\t%s+1\n",	v);
		printf("\tEORA\t%s\n",		v);
}

void	TAB()
{
		printf("\tTAB\n");
}
void	TBA()
{
		printf("\tTBA\n");
}
void	CLRA()
{
		printf("\tCLRA\n");
}
void	CLRB()
{
		printf("\tCLRB\n");
}
void	CLRD()
{
		CLRB();
		CLRA();
}
void	CLR_V(char *v)
{
		printf("\tCLR\t_%s+1\n", v);
		printf("\tCLR\t_%s\n", v);
}
void	CLR_V1(char *v)
{
		printf("\tCLR\t_%s+1\n", v);
}
void	CLR_V0(char *v)
{
		printf("\tCLR\t_%s\n", v);
}
void	CLR_X(int v)
{
		printf("\tCLR\t%d,X\n",v+1);
		printf("\tCLR\t%d,X\n",v);
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
void	STD_V(char *str)
{
		printf("\tSTAB\t_%s+1\n",	str);
		printf("\tSTAA\t_%s\n",		str);
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
void	LDAB_I(int v)
{
		printf("\tLDAB\t#%d\n",v);
}
void	LDAA_V(char *str)
{
		printf("\tLDAB\t_%s\n",str);
}
void	LDAB_V(char *str)
{
		printf("\tLDAB\t_%s+1\n",str);
}
void	LDAB_X(int v)
{
		printf("\tLDAB\t%d,X\n",v);
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
}
void	LDX_I(int v)
{
		printf("\tLDX\t#%d\n",v);
}
void	LDX_X(int v)
{
		printf("\tLDX\t%d,X\n",v);
}
void	LDX_V(char *str)
{
		printf("\tLDX\t_%s\n",str);
}
void	LDX_L(char *str)
{
		printf("\tLDX\t%s\n",str);
}
void	LDX_IL(char *str)
{
		printf("\tLDX\t#%s\n",str);
}
void	STX_V(char *v)
{
		printf("\tSTX\t_%s\n",	v);
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
}
void	JSR_X(int v)
{
		printf("\tJSR\t%d,X\n",v);
}
void	MUL256()
{
		printf("\tTBA\n");
		CLRB();
}

void
Bxx(char *cc, char *label)
{
		printf("\tB%s\t%s\n",cc,label);
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
		STD_L("TDXWK");
		LDX_L("TDXWK");
#else
		// 26cycle 7bytes
		PSHB();
		PSHA();
		TSX();
		LDX_X(0);
		INS();
		INS();
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

void	gen_store_var(Node *node)
{
		STD_V(node->str);
}

void gen_expr(Node *node);

//
//	var:offset) または var(offset)のアドレスを得る(IX+戻り値)
//		ex. A:1)
//				LDX	_A
//				(1が戻ってくる)
//
int
gen_array_address(Node *node)
{
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; gen_array_address error\n");
	}
	if(node->kind==ND_ARRAY1){						// 1バイト間接モード
		if(isNUM(node->lhs)){					// オフセットが定数
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
				printf("\tINC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tDEC\t_%s\n",node->str);
				return	offset-256;
#ifndef	O_SPEED
			}else if(offset>=512 && offset<1024){
				printf("\tINC\t_%s\n",node->str);
				printf("\tINC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tDEC\t_%s\n",node->str);
				printf("\tDEC\t_%s\n",node->str);
				return	offset-512;
#endif
			}else if(offset<0 && offset>=-256){
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				return	offset+256;
#ifndef	O_SPEED
			}else if(offset<-256 && offset>=-512){
				printf("\tDEC\t_%s\n",node->str);
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				printf("\tINC\t_%s\n",node->str);
				return	offset+512;
#endif
			}
		}
		gen_expr(node->lhs);	// 添字の計算をして
		ADD_V(node->str);		// 配列の添字はAccABにある
		TDX();					// TFR D,X
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
				printf("\tINC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tDEC\t_%s\n",node->str);
				return	offset*2-256;
#ifndef		O_SPEED
			}else if(offset>=256 && offset<511){
				printf("\tINC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tDEC\t_%s\n",node->str);
				return	offset*2-512;
#endif
			}else if(offset<0 && offset>=-127){
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				return	offset*2+256;
#ifndef	O_SPEED
			}else if(offset<-127 && offset>=-255){
				printf("\tDEC\t_%s\n",node->str);
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				printf("\tINC\t_%s\n",node->str);
				return	offset*2+512;
#endif
			}
		}
		gen_expr(node->lhs);	// 添字の計算をして
		ASLD();
		ADD_V(node->str);		// 配列の添字はAccABにある
		TDX();					// TFR D,X
		return 0;
	}
	error("; what's happen? gen_array_address\n");
	return 0;
}

void gen_expr(Node *node);

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
;	printf("; gen_compare :");print_nodes_ln(node);
	char	*if_false = new_label();
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
;		printf("; gen_compare expr/var >=,< NUM");print_nodes_ln(node);
		if(isVAR(node->lhs)){
			TST_V0(node->lhs->str);
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
		CLRA();
		return;
	}else if(isNUMorVAR(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){	// ==,!= はCPXで
;		printf("; gen_compare expr ==,!= NUM");print_nodes_ln(node);
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
		}else{
			LDX_V(node->lhs->str);
		}
		if(isNUM(node->rhs)){
			if(node->rhs->val){			// LDXはZフラグが立つので0との比較は省略
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
		CLRA();
		return;
	}else if(isNUM(node->rhs)){					// 左辺は式、右辺は定数
;		printf("; gen_compare expr ==,!= NUM");print_nodes_ln(node);
		gen_expr(node->lhs);					// AccABに結果がある
		// 以下、AccAB(lhs)とNUM(rhs)を比較
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_false);
					CMPA_I(high(node->rhs->val));
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_true);
					CMPA_I(high(node->rhs->val));
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_I(node->rhs->val);
					Bxx("LT",if_false);
					break;
		case ND_GT:	SUB_I(node->rhs->val);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	SUB_I(node->rhs->val);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LT:	SUB_I(node->rhs->val);
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
		CLRA();
		return;
	}else if(isVAR(node->rhs)){					// 左辺は式、右辺は変数
;		printf("; gen_compare expr ?? VAR");print_nodes_ln(node);
		gen_expr(node->lhs);					// AccABに結果がある
		// 以下、AccAB(lhs)とVAR(rhs)を比較
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	//printf("; gen_compare ND_EQ expr?expr\n");
					CMPB_V(node->rhs->str);
					Bxx("NE",if_false);
					CMPA_V(node->rhs->str);
					Bxx("NE",if_false);
					break;
		case ND_NE:	//printf("; gen_compare ND_NE expr?expr\n");
					CMPB_V(node->rhs->str);
					Bxx("NE",if_true);
					CMPA_V(node->rhs->str);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	//printf("; gen_compare ND_GE expr?expr\n");
					SUB_V(node->rhs->str);		// lhs >= rhs ?
					Bxx("LT",if_false);			// lhs <  rhs -> false, then lhs>=rhs
					break;
		case ND_GT:	//printf("; gen_compare ND_GT expr?expr\n");
					// 128>-128? -> 128-(-128) -> 0080+0080 -> 0x0100 N=0,V=0 is true
					SUB_V(node->rhs->str);		// lhs > rhs ?
					Bxx("LT",if_false);			// lhs < rhs -> false
					Bxx("NE",if_true);			// lhs > rhs -> true
					TSTB();
					Bxx("EQ",if_false);			// lhs = rhs -> false
					break;
		case ND_LE:	//printf("; gen_compare ND_LE expr?expr\n");
					SUB_V(node->rhs->str);		// lhs <= rhs?
					Bxx("GT",if_false);			// lhs < rhs -> true, then lhs>=rhs
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_false);			// lhs != rhs -> false
					break;
		case ND_LT:	//printf("; gen_compare ND_LT expr?expr\n");
					SUB_V(node->rhs->str);		// lhs > rhs?
					Bxx("GE",if_false);			// lhs =< rhs -> false
					break;
		default:
					error("gen_compare expr?VAR ND_cc not known\n");
		}
		LABEL(if_true);
		LDAB_I(1);
		SKIP1();
		LABEL(if_false);
		CLRB();
		CLRA();
		return;
	}else{
;		printf("; gen_compare expr ?? expr");print_nodes_ln(node);
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
//	printf("; gen_branch_if_false %s :",label);print_nodes_ln(node);
	char	*if_false = new_label();
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(node->lhs)){
			TST_V0(node->lhs->str);
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
			if(node->rhs->val){			// LDXはZフラグが立つので0との比較は省略
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
	}else if(isARRAY2(node->lhs) && isNUMorVAR(node->rhs) && isEQorNE(node)){// 配列との ==,!= はCPXで
		int offset = gen_array_address(node->lhs);		// 配列アドレスがIXに入っている
		LDX_X(offset);
		if(isNUM(node->rhs)){
			if(node->rhs->val){			// LDXはZフラグが立つので0との比較は省略
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
	}else if(isNUM(node->rhs)){					// 左辺は式、右辺は定数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_false);
					CMPA_I(high(node->rhs->val));
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_true);
					CMPA_I(high(node->rhs->val));
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_I(node->rhs->val);		// >=
					Bxx("LT",if_false);
					break;
		case ND_GT:	SUB_I(node->rhs->val);		// >
					Bxx("GT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	SUB_I(node->rhs->val);		// <=
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LT:	SUB_I(node->rhs->val);		// <
					Bxx("GE",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_true);
		JMP(label);
		LABEL(if_false);
		return;
	}else if(isVAR(node->rhs)){					// 左辺は式、右辺は変数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_V(node->rhs->str);
					Bxx("NE",if_false);
					CMPA_V(node->rhs->str);
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_V(node->rhs->str);
					Bxx("NE",if_true);
					CMPA_V(node->rhs->str);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_V(node->rhs->str);		// >=
					Bxx("LT",if_false);
					break;
		case ND_GT:	SUB_V(node->rhs->str);		// >
					Bxx("GT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LE:	SUB_V(node->rhs->str);		// <=
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LT:	SUB_V(node->rhs->str);		// <
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
			TST_V0(node->lhs->str);
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
			if(node->rhs->val){			// LDXはZフラグが立つので0との比較は省略
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
	}else if(isNUM(node->rhs)){					// 左辺は式、右辺は定数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_false);
					CMPA_I(high(node->rhs->val));
					Bxx("EQ",if_true);
					break;
		case ND_NE:	CMPB_I(low(node->rhs->val));
					Bxx("NE",if_true);
					CMPA_I(high(node->rhs->val));
					Bxx("NE",if_true);
					break;
		case ND_GE:	SUB_I(node->rhs->val);
					Bxx("GE",if_false);
					break;
		case ND_GT:	SUB_I(node->rhs->val);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_true);
					break;
		case ND_LE:	SUB_I(node->rhs->val);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_true);
					break;
		case ND_LT:	SUB_I(node->rhs->val);
					Bxx("LT",if_true);
					break;
		default:
					error("Bxx not known\n");
		}
		LABEL(if_false);
		JMP(label);
		LABEL(if_true);
		return;
	}else if(isVAR(node->rhs)){					// 左辺は式、右辺は変数
		gen_expr(node->lhs);					// AccABに結果がある
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_V(node->rhs->str);
					Bxx("NE",if_false);
					CMPA_V(node->rhs->str);
					Bxx("EQ",if_true);
					break;
		case ND_NE:	CMPB_V(node->rhs->str);
					Bxx("NE",if_true);
					CMPA_V(node->rhs->str);
					Bxx("NE",if_true);
					break;
		case ND_GE:	SUB_V(node->rhs->str);
					Bxx("GE",if_true);
					break;
		case ND_GT:	SUB_V(node->rhs->str);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_true);
					break;
		case ND_LE:	SUB_V(node->rhs->str);
					Bxx("GT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_true);
					break;
		case ND_LT:	SUB_V(node->rhs->str);
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
	case ND_ARRAY1:
//			printf("; gen array1\n");
//			printf("; ");print_nodes_ln(node->lhs);
			if(isNUM(node->lhs) && node->lhs->val>=0 && node->lhs->val<=255){	// 添字が小さな定数
				LDX_V(node->str);
				int offset = (node->lhs->val);
				LDAB_X(offset);
				CLRA();
				return;
			}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-4){
				LDX_V(node->str);
				if(node->lhs->val<0){ DEX(); }
				if(node->lhs->val<-1){ DEX(); }
				if(node->lhs->val<-2){ DEX(); }
				if(node->lhs->val<-3){ DEX(); }
				LDAB_X(0);
				CLRA();
				return;
			}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-256){
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				LDAB_X(256+(node->lhs->val));
				CLRA();
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
			CLRA();
#endif
			return;
	case ND_ARRAY2:
//			printf("; gen array2\n");
//			printf("; ");print_nodes_ln(node->lhs);
			if(isNUM(node->lhs) && node->lhs->val>=0 && node->lhs->val<=127){	// 添字が小さな定数
				LDX_V(node->str);
				int offset = (node->lhs->val)*2;
				LDD_X(offset);
				return;
			}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-2){
				LDX_V(node->str);
				if(node->lhs->val<0){ DEX();DEX(); }
				if(node->lhs->val<-1){ DEX();DEX(); }
				LDD_X(0);
				return;
			}else if(isNUM(node->lhs) && node->lhs->val<0 && node->lhs->val>=-127){
				printf("\tDEC\t_%s\n",node->str);
				LDX_V(node->str);
				printf("\tINC\t_%s\n",node->str);
				LDD_X(256+(node->lhs->val)*2);
				return;
			}
			gen_expr(node->lhs);		// 添字の計算
#if	0
			LDX_V(node->str);
			JSR("ADX2");
			LDD_X(0);
#else
			ASLD();
			ADD_V(node->str);			// 変数アドレスをLVに
			TDX();
			LDD_X(0);
#endif
			return;
	// 数値
	case ND_NUM:
			LDD_I(node->val);
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
			STX_L("_CPXWK");
			LDD_L("_CPXWK");
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
			gen_expr(node->lhs);
			if(node->rhs->kind==ND_NUM){	
				ADD_I(node->rhs->val);
			}else if (node->rhs->kind==ND_VAR){
				ADD_V(node->rhs->str);
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
					STD_L("TDXWK");
					ASLD_N(2);
					ADD_L("TDXWK");
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
					STD_L("TDXWK");
					ASLD_N(1);
					ADD_L("TDXWK");
					if(node->rhs->val==12){
						ASLD();
						ASLD();
					}else if(node->rhs->val==6){
						ASLD();
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
							INS();
							INS();
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
	case ND_LINENUM:
//		printf("*\t");print_line(node->str);printf("\n");
		LABEL(new_line_label(node->val));
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
				int offset = gen_array_address(lhs);
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
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					CLR1_X(offset);
					return;
				}else if(isNUM(rhs)){
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					LDAB_I(rhs->val);
					STAB_X(offset);
					return;
				}else if(isVAR(rhs)){
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					LDAB_V(rhs->str);
					STAB_X(offset);
				return;
				}
				gen_expr(lhs->lhs);			// calculate subscript
				ADD_V(lhs->str);				// 左辺のアドレスはDにある
				PSHD();
				gen_expr(rhs);
				TSX();
				LDX_X(0);
				INS();
				INS();
				STAB_X(0);
				return;
			}else if(lhs->kind==ND_ARRAY2){
				//  (ND_ASSIGN (ND_ARRAY2 str=N (ND_VAR str=I)) (ND_NUM 0))
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					CLR_X(offset);
					return;
				}else if(isNUM(rhs)){
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					LDD_I(rhs->val);
					STD_X(offset);
					return;
				}else if(isVAR(rhs)){
					int	offset = gen_array_address(lhs);	// X=adrs, offset=subscript
					LDD_V(rhs->str);
					STD_X(offset);
					return;
				}
				gen_expr(lhs->lhs);			// calculate subscript
				ASLD();
				ADD_V(lhs->str);				// 左辺のアドレスはDにある
				PSHD();
				gen_expr(rhs);
				TSX();
				LDX_X(0);
				INS();
				INS();
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
				printf("\tBNE\t%s\n",IF_TRUE);
				printf("\tJMP\t%s\n",NEXT_LINE);
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
			printf("\tABA\n");
			printf("\tBNE\t%s\n",IF_TRUE);
			printf("\tBCS\t%s\n",IF_TRUE);
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
				printf("\tBEQ\t%s\n",IF_FALSE);
				printf("\tJMP\t%s\n",GOTOLINE);
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
		return;
	case ND_GOTO:
		if(node->lhs->kind!=ND_NUM){
			error("GOTO linenumber not constant\t");
			return;
		}
		JMP(new_line_label(node->val));
		return;
	case ND_GOSUB:
		if(node->lhs->kind!=ND_NUM){
			error("GOSUB linenumber not constant\t");
			return;
		}
		JSR(new_line_label(node->val));
		return;
	case ND_DO:
		LABEL(new_do_label(node->val));
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
			printf("\tABA\n");
			printf("\tBNE\t%s\n",DO_SKIP);
			printf("\tBCS\t%s\n",DO_SKIP);
			JMP(DO_LOOP);
			LABEL(DO_SKIP);
		}
		return;
	case ND_FOR: {
			// (ND_FOR J (ND_SETVAR J 0 ) 10 )
			char	*FOR_LABEL=new_for_label(node->val);
			char	*TO_LABEL=new_to_label(node->str);
			FORTO_RMB[(node->str)[0]-'A'] = TO_LABEL;
			gen_stmt(node->lhs);
			if(isNUM(node->rhs)){
				LDX_I(node->rhs->val);
				STX_L(TO_LABEL);
			}else if(isVAR(node->rhs)){
				LDX_V(node->rhs->str);
				STX_L(TO_LABEL);
			}else{
				gen_expr(node->rhs);
				STD_L(TO_LABEL);
			}
			LABEL(FOR_LABEL);
		}
		return;
	case ND_NEXT: {
		// (ND_NEXT J (+ (ND_VAR J) 1 ))
		char	*FOR_LABEL=new_for_label(node->val);
		char	*TO_LABEL=new_to_label(node->str);
		char	*NEXT_LABEL=new_label();
		char	*to_FOR=new_label();
		Node	*node_step = new_unary(ND_SETVAR,node->lhs);
		node_step->str = node->str;
		Node	*opt = node_opt(node_step);
		printf(";  ");print_nodes_ln(node_step);
		printf(";=>");print_nodes_ln(opt);
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
		SUB_L(TO_LABEL);
		Bxx("GT",NEXT_LABEL);
		Bxx("NE",to_FOR);
		TSTB();
		Bxx("NE",NEXT_LABEL);
		LABEL(to_FOR);
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
		gen_expr(node->lhs);
		printf("\tTBA\n");
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
	case ND_SETVAR:
		if(isNUM(node->lhs)){
			LDX_I(node->lhs->val);
			STX_V(node->str);
		}else if(isVAR(node->lhs)){
			LDX_V(node->lhs->str);
			STX_V(node->str);
		}else if(isARRAY(node->lhs)){			// Z=A(I) or Z=A:I)
			printf("; ND_SETVAR ");print_nodes_ln(node);
			int offset = gen_array_address(node->lhs);		// IXにA(I) or A:I)のアドレスが入る
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
		return;
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
	for_to_count = 0;
	string_count = 0;
	printf("*\n");
	printf("* generated by GAME68 compiler\n");
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
	for(int i=0; i<string_count; i++){
		LABEL(STRING_FCC[i].label);
		printf("*\t%s\n",STRING_FCC[i].orig);
		printf("%s",STRING_FCC[i].str);
		printf("\tFCB\t0\n");
	}
	for(int i=0; i<'Z'-'A'; i++){
		if(FORTO_RMB[i]){
			RMB(FORTO_RMB[i],2);
		}
	}
	printf("\tINCLUDE\tgamecc.asm\n");
	printf("_LPGEND\tEQU\t*\n");
	printf("\tEND\tmain\n");
}

