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

typedef	enum {
	LOC_NONE=0,
	LOC_USED,
	LOC_CONST,
	LOC_LVAR,
	LOC_LARRAY1,
	LOC_LARRAY2,
	LOC_LADRS,			// アドレス計算の結果が入っている
} loc_type_t;

typedef	struct	{
	loc_type_t	kind;
	char		*reg;	// A|B|D|X or temporary area name (TMP0-7,LADRS),変数名は入らない
	int			val;	// const or offset // X(I+offset)
	char		*var1;	// array name				// X in X(I+offset)
	char		*var2;	// array subscript name		// I in X(I+offset)
}	loc_table_t;	// location value
loc_table_t	loc_table[256];	// freeするなら16ぐらいでいいかもA
#define	loc_table_size	(int)(sizeof(loc_table)/sizeof(loc_table_t))
int	loc_next = 0;

void	dump_loc_table()
{
	printf("; debug dump_loc_table()\n");
	for(int i=0; i<loc_table_size; i++){
		loc_table_t	*p = &(loc_table[i]);
		if(p->kind>0){
			printf("; %d ",i);
			switch(p->kind){
			case LOC_USED:	printf("LOC_USED   : used for tempoary save\n");
							break;
			case LOC_CONST:	{
								if(strcmp(p->reg,"D")==0 && (p->val>32767)){
									printf("LOC_CONST  : reg %s, %d\n",p->reg,p->val-65536);
								}else{
									printf("LOC_CONST  : reg %s, %d\n",p->reg,p->val);
								}
							}
							break;
			case LOC_LVAR:	printf("LOC_LVAR   : reg %s, var %s\n",p->reg,p->var1);
							break;
			case LOC_LARRAY1:printf("LOC_LARRAY1: reg %s, var %s:%s+%d)\n",
													p->reg,p->var1,p->var2,p->val);
							break;
			case LOC_LARRAY2:printf("LOC_LARRAY2: reg %s, var:%s(%s+%d)\n",
													p->reg,p->var1,p->var2,p->val);
							break;
			case LOC_LADRS:	printf("LOC_LADRS   : reg %s\n",p->reg);
							break;
			default:
				error("; unknown loc_table[%d].kind=%d\n",i,p->kind);
			}
			fflush(stdout);
		}
	}
}
//
//	全てのloc val情報を消す
//
void	lv_free_all()
{
//	printf("; lv_free_all()\n");
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		p->kind = LOC_NONE;
		p->reg = "";
		p->var1 = "";
		p->var2 = "";
	}
	loc_next = 0;
}
//
//	レジスタrの情報を消す
//
void	lv_free_reg(char *r)
{
//	printf("; lv_free_reg %s\n",r);fflush(stdout);
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind!=LOC_NONE)
		&& (strcmp(p->reg,r)==0)){
			p->kind = LOC_NONE;
		}
	}
//	printf("; lv_free_reg end %s\n",r);fflush(stdout);
}
void	lv_free_reg_A()
{
	lv_free_reg("A");
}
void	lv_free_reg_B()
{
	lv_free_reg("B");
}
void	lv_free_reg_AB()
{
	lv_free_reg_A();
	lv_free_reg_B();
}
void	lv_free_reg_D()
{
	lv_free_reg("D");
}
void	lv_free_reg_ABD()
{
	lv_free_reg_AB();
	lv_free_reg_D();
}
void	lv_free_reg_X()
{
	lv_free_reg("X");
}
void	lv_free_reg_ABDX()
{
	lv_free_reg_ABD();
	lv_free_reg_X();
}
//
//	 変数vの情報を消す
//
void	lv_free_var(char *v)
{
//	printf("; lv_free_var %s\n",v);fflush(stdout);
	if((strcmp(v,"MOD")==0)
	|| (strncmp(v,"TMP",3)==0)){
		return;
	}
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if(p->kind==LOC_NONE){
			continue;
		}
//		printf("; lv_free_var %s, search %d\n",v,i);fflush(stdout);
		if((strcmp(p->var1,v)==0)
		|| (strcmp(p->var2,v)==0)){
			p->kind = LOC_NONE;
		}
	}
//	printf("; lv_free_var end\n");fflush(stdout);
}
//
//	 一時変数TMPxの情報を消す
//
void	lv_free_tmp(char *v)
{
//	printf("; lv_free_var %s\n",v);fflush(stdout);
	if((strcmp(v,"MOD")==0)
	|| (strncmp(v,"TMP",3)!=0)){
		return;
	}
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if(p->kind==LOC_NONE){
			continue;
		}
//		printf("; lv_free_var %s, search %d\n",v,i);fflush(stdout);
		if((strcmp(p->var1,v)==0)
		|| (strcmp(p->var2,v)==0)){
			p->kind = LOC_NONE;
		}
		if(strcmp(p->reg,v)==0){
			p->kind = LOC_NONE;
		}
	}
//	printf("; lv_free_var end\n");fflush(stdout);
}
//
//	空きを探してその番号を返す。空きがなければエラー（ひどい）
//	(たぶん、空きはあるはず）
//
int	lv_search_free()
{
//	printf("; lv_search_free start\n");fflush(stdout);
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if(p->kind==LOC_NONE){
//			printf("; lv_search_free found %d\n",i);fflush(stdout);
			return	i;
		}
	}
	error("; lv_search_free: no free space\n");
	return -1;
}
//
//
//
int	lv_search_tmp(char *tmp)
{
//	printf("; lv_search_tmp %s start\n",tmp);fflush(stdout);
	for(int i=0; i<loc_table_size; i++){
		if((loc_table[i].kind!=LOC_NONE)
		&& (strcmp(loc_table[i].reg,tmp)==0)){
//			printf("; lv_search_tmp %s found %d\n",tmp,i);fflush(stdout);
			return	1;
		}
	}
//	printf("; lv_search_tmp %s not found\n",tmp);fflush(stdout);
	return	0;
}
//
//	使われていないTMPを探し、TMP名を返す(TMP0-TMP15)
//	全部使われていたら、落ちる(ひどい)
//
char *lv_search_free_tmp()
{
//	printf("; lv_search_free_tmp\n");fflush(stdout);
	char	*tmp = calloc(1,10);
	for(int i=0; i<15; i++){
		sprintf(tmp,"TMP%d",i);
		if(!lv_search_tmp(tmp)){
//			printf("; lv_search_free_tmp found %s\n",tmp);fflush(stdout);
			i = lv_search_free();
			loc_table[i].reg = tmp;
			loc_table[i].kind = LOC_USED;
			return	tmp;
		}
	}
//	printf("; lv_search_free_tmp not found\n");fflush(stdout);
//	sprintf(tmp,"TMP%d",rand()%6);
	error("; no free tmp register");
//	lv_free_reg(tmp);
//	printf("; lv_search_free_tmp return %s\n",tmp);fflush(stdout);
	return	tmp;
}
//
//	レジスタrは未使用（有効な値が入っていない）か？
//
int	lv_is_free(char *r)
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind!=LOC_NONE)
		&& (strcmp(p->reg,r)==0)){
//			printf("; lv_search_ts found %d\n",i);
			return	0;
		}
	}
//	printf("; lv_search_ts end\n");
	return	1;
}
//
//	種別tがレジスタrに入っているか？
//
int	lv_search_ts(loc_type_t kind,char *r)
{
//	printf("; lv_search_ts %s\n",r);
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==kind)
		&& (strcmp(p->reg,r)==0)){
//			printf("; lv_search_ts found %d\n",i);
			return	i;
		}
	}
//	printf("; lv_search_ts end\n");
	return	-1;
}
//
//	レジスタrに登録されている定数があるか?
//		なければ0を返す
//		あれば、valに定数を入れて!0を返す
//
int	lv_search_reg_const(char *r,int *val)
{
//	printf("; lv_search_reg_const %s?\n",r);fflush(stdout);
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
//		printf("; lv_search_reg_const %s i=%d\n",r,i);fflush(stdout);
//		printf(";   loc_table[%d].kind=%d\n",i,p->kind);fflush(stdout);
		if((p->kind==LOC_CONST)
		&& (strcmp(p->reg,r)==0)){
//			printf("; lv_search_reg_const %s found\n",r);fflush(stdout);
			*val = p->val;
//			printf("; lv_search_reg_const %s found %d\n",r,*val);fflush(stdout);
			return	1;
		}
	}
//	printf("; lv_search_reg_const %s not found\n",r);fflush(stdout);
	return	0;
}
//
//	レジスタr:ABDXに定数valがあるか
//		あれば、そのレジスタ名(char)を返す。なければ0を返す
//
int	lv_search_reg_const_val(char *r,int val)
{
//	printf("; lv_search_reg_const_val %s=%d?\n",r,val);fflush(stdout);
	char *s = calloc(1,10);
	s[0] = r[0]; s[1]=0;
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
//		printf("; lv_search_reg_const_val %s %d i=%d\n",r,val,i);fflush(stdout);
		if((p->kind==LOC_CONST)
		&& (strcmp(p->reg,s)==0)
		&& (p->val==val)){
//			printf("; lv_search_reg_const_val %s=%d fund %s\n",r,val,s);fflush(stdout);
			return	s[0];
		}
	}
//	printf("; lv_search_reg_const_val %s=%d not found\n",r,val);fflush(stdout);
	return	0;
}
//
//	レジスタr:ABDXに変数varがあるか
//		あれば、その非0を返す。なければ0を返す
//
int	lv_search_reg_var(char *r,char *var)
{
//	printf("; lv_search_reg_var %s %s\n",r,var);
#if	0
//	printf("; lv_search_reg_var current registr %s var=",r);fflush(stdout);
	for(int i=0;i<loc_table_size;i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==LOC_LVAR)
		&& (strcmp(p->reg,r)==0)){
			printf("%s,",p->var1);fflush(stdout);
		}
	}
	printf("\n");
#endif
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==LOC_LVAR)
		&& (strcmp(p->reg,r)==0)
		&& (strcmp(p->var1,var)==0)){
//			printf("; lv_search_reg_var found %d\n",i);
			return	1;
		}
	}
//	printf("; lv_search_reg_var %s != %s\n",r,var);
	return	0;
}
//
//	レジスタIX/TMP/LDARSに配列var1,var2があるか
//		あれば、そのレジスタ名(char *)とoffsetを返す。なければNULLを返す
//		var2は空文字列を許す
//
char *lv_search_reg_any_array(char *var1,char *var2,int *offset)
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==LOC_LARRAY1 || p->kind==LOC_LARRAY2)
		&& (strcmp(p->var1,var1)==0)
		&& (strcmp(p->var2,var2)==0)){
			*offset = p->val;
			return	p->reg;
		}
	}
	return	NULL;
}
//
//	レジスタXに配列var1,var2,offsetがあるか。
//		あれば、1を返す。なければ0を返す
//
int	lv_search_reg_X_array(loc_type_t kind,char *var1,char *var2,int offset)
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==kind)
		&& (strcmp(p->reg,"X")==0)
		&& (p->val==offset)
		&& (strcmp(p->var1,var1)==0)
		&& (strcmp(p->var2,var2)==0)){
			return	1;
		}
	}
	return	0;
}
//
//	レジスタTMP?に配列var1,var2,offsetがあるか。
//		あれば、レジスタ名を返す。なければNULLを返す
//
char *lv_search_tmp_array(loc_type_t kind,char *var1,char *var2,int offset)
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==kind)
		&& (strncmp(p->reg,"TMP",3)==0)
		&& (p->val==offset)
		&& (strcmp(p->var1,var1)==0)
		&& (strcmp(p->var2,var2)==0)){
			return	p->reg;
		}
	}
	return	NULL;
}
//
//	レジスタrに定数値情報を登録する。rはA/B/D/Xのいずれか。
//	（rに定数値情報があれば上書きする）
//
void	lv_add_reg_const(char *r, int val)
{
//	printf("; lv_add_reg_const %s=%d\n",r,val);
	int	i =lv_search_ts(LOC_CONST,r);

	if(i==-1){
//		printf("; lv_add_reg_const %s,%d not found\n",r,val);
		i = lv_search_free();
	}
//	printf("; lv_add_reg_const %s=%d store to %d\n",r,val,i);
	loc_table[i].kind = LOC_CONST;
	loc_table[i].reg  = r;
	loc_table[i].val  = val;
//	dump_loc_table();
}
//
//	レジスタrに変数情報を追加する。上書きしない
//
void	lv_add_reg_var(char *r, char *var)
{
	if(strcmp(var,"MOD")==0){
		return;
	}
	if(strcmp(r,"D")==0 && strncmp(var,"TMP",3)==0){
		return;
	}
//	printf("; lv_add_reg_var %s=%s\n",r,var);fflush(stdout);
	int	i = lv_search_free();
//	printf("; lv_add_reg_var store %s to %d\n",r,i);fflush(stdout);
	loc_table[i].kind = LOC_LVAR;
	loc_table[i].reg  = r;
	loc_table[i].var1 = var;
#if	0
	printf("; lv_add_reg_var current registr %s var=",r);fflush(stdout);
	for(int i=0;i<loc_table_size;i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==LOC_LVAR)
		&& (strcmp(p->reg,r)==0)){
			printf("%s,",p->var1);fflush(stdout);
		}
	}
	printf("\n");
#endif
}
//
//	レジスタrに配列変数情報を単に追加する
//
void	lv_add_reg_array_nofree(char *r, loc_type_t kind, char *var1, char *var2, int offset)
{
//	printf("; lv_add_reg_array %s=%s,%s,%d\n",r,var1,var2,offset);fflush(stdout);
	if((kind!=LOC_LARRAY1)&&(kind!=LOC_LARRAY2)){
		error("; lv_add_reg_array: illegal kind %d\n",kind);
	}
	if((strcmp(r,"X")==0)
	|| (strncmp(r,"TMP",3)==0)
	|| (strcmp(r,"LADRS")==0)){
		int i = lv_search_free();
		loc_table[i].kind = kind;
		loc_table[i].reg  = r;
		loc_table[i].var1 = var1;
		loc_table[i].var2 = var2;
		loc_table[i].val  = offset;
//		printf("; lv_add_reg_array %s=%s%s%s+%d) store to %d\n",
//							r,var1,(kind==LOC_LARRAY1)?":":"(",var2,offset,i);fflush(stdout);
		return;
	}
	error("; ld_add_array_r can't add to register A,B,D \n");
}
//
//	レジスタrに配列変数情報を追加する。既にrに情報があれば消してから追加する
//
void	lv_add_reg_array(char *r, loc_type_t kind, char *var1, char *var2, int offset)
{
//	printf("; lv_add_reg_array %s=%s,%s,%d\n",r,var1,var2,offset);fflush(stdout);
	if((kind!=LOC_LARRAY1)&&(kind!=LOC_LARRAY2)){
		error("; lv_add_reg_array: illegal kind %d\n",kind);
	}
	lv_free_reg(r);
	if((strcmp(r,"X")==0)
	|| (strncmp(r,"TMP",3)==0)
	|| (strcmp(r,"LADRS")==0)){
		int i = lv_search_ts(kind,r);
		if(i==-1){
			i = lv_search_free();
		}
		loc_table[i].kind = kind;
		loc_table[i].reg  = r;
		loc_table[i].var1 = var1;
		loc_table[i].var2 = var2;
		loc_table[i].val  = offset;
//		printf("; lv_add_reg_array %s=%s%s%s+%d) store to %d\n",
//							r,var1,(kind==LOC_LARRAY1)?":":"(",var2,offset,i);fflush(stdout);
		return;
	}
	error("; ld_add_array_r can't add to register A,B,D \n");
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

	sprintf(label,"LT%d",v);
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

uint16_t	word(int x)
{
	return	(x & 0x0ffff);
}

uint16_t	make_word(int a,int b)
{
	return	word((low(a)<<8) + low(b));
}

//
//	reg "A" なら "B" を "B" なら "A" を返す
//
char	*pair_reg(char *r)
{
		if(strcmp(r,"A")==0){
			return	"B";
		}else if(strcmp(r,"B")==0){
			return	"A";
		}
		error("; pair_reg unknown register '%s'\n",r);
		return NULL;
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
//
//		Acc r の値をvに更新する
//
void	UPDATEr(char *r,int v)
{
		v = low(v);
		lv_add_reg_const(r,v);
		char *pair = pair_reg(r);
		int pval;
		if(lv_search_reg_const(pair,&pval)){				// ペアの値が決まってればDもわかる
			if(strcmp(r,"A")==0){
				lv_add_reg_const("D",make_word(v,pval));
			}else{
				lv_add_reg_const("D",make_word(pval,v));
			}
			return;
		}
		lv_free_reg("D");
}
void	INCDECr(char *r,int v)
{
		switch(v){
		case -1:printf("\tDEC%s\n",r);break;
		case 1: printf("\tINC%s\n",r);break;
		default:
			error("; INCDECr %d\n",v);
		}
		int val;
		if(lv_search_reg_const(r,&val)){					// rの値が既知なら、計算できる
			val = low(val+v);
			UPDATEr(r,val);
			return;
		}
		lv_free_reg("D");									// 未知なのでDも消す
		lv_free_reg(r);
}
void	INCr(char *r)
{
		INCDECr(r,1);
}
void	INCA()
{
		INCr("A");
}
void	INCB()
{
		INCr("B");
}
void	DECr(char *r)
{
		INCDECr(r,-1);
}
void	DECA()
{
		DECr("A");
}
void	DECB()
{
		DECr("B");
}
void	INC_V0(char *v)
{
		printf("\tINC\t_%s\n",v);
		lv_free_var(v);
}
void	DEC_V0(char *v)
{
		printf("\tDEC\t_%s\n",v);
		lv_free_var(v);
}
void	INC_V1(char *v)
{
		printf("\tINC\t_%s+1\n",v);
		lv_free_var(v);
}
void	DEC_V1(char *v)
{
		printf("\tDEC\t_%s+1\n",v);
		lv_free_var(v);
}
void	TST_V1(char	*str)
{
		printf("\tTST\t_%s+1\n",str);
}
void	TST_V0(char	*str)
{
		printf("\tTST\t_%s\n",str);
}
void	TSX_nofree()
{
		printf("\tTSX\n");
}
void	TSX()
{
		printf("\tTSX\n");
		lv_free_reg_X();
}
void	INX()
{
		printf("\tINX\n");
		int val;
		if(lv_search_reg_const("X",&val)){
			lv_add_reg_const("X",word(val+1));
			return;
		}
		lv_free_reg_X();
}
void	INX2()
{
		INX();
		INX();
}
void	DEX()
{
		printf("\tDEX\n");
		int val;
		if(lv_search_reg_const("X",&val)){
			lv_add_reg_const("X",word(val-1));
			return;
		}
		lv_free_reg_X();
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
		lv_free_reg("D");
		lv_free_reg("A");
}
void	PULB()
{
		printf("\tPULB\n");
		lv_free_reg("D");
		lv_free_reg("B");
}
void	PULD()
{
		PULA();
		PULB();
}

void	LSRB()
{
		int v1,v2;
		printf("\tLSRB\n");
		if(lv_search_reg_const("B",&v1)){
            lv_add_reg_const("B",low(v1>>2));
			if(lv_search_reg_const("A",&v2)){
				lv_add_reg_const("D",make_word(v2,v1>>2));
			}
			return;
		}
		lv_free_reg_B();
}
void	ASLD()
{
		int e;
		int val;
		if((e=lv_search_reg_const("D",&val))
		&& val==0){
			return;
		}
		printf("\tASLB\n");
		printf("\tROLA\n");
		if(e){
			lv_add_reg_const("A",high(val<<2));
			lv_add_reg_const("B",low(val<<2));
			lv_add_reg_const("D",word(val<<2));
			return;
		}
		lv_free_reg_ABD();
}
void	ASLD_N(int n)
{
		while(n-->0){
			ASLD();
		}
}
void	ASRD()
{
		printf("\tASRA\n");
		printf("\tRORB\n");
		lv_free_reg_ABD();
}
void	ASRD_N(int n)
{
		while(n-->0){
			ASRD();
		}
}

void	NEGD()
{
		printf("\tNEGA\n");
		printf("\tNEGB\n");
		printf("\tSBCA\t#0\n");
		int val;
		if(lv_search_reg_const("D",&val)){
			lv_add_reg_const("B",low(word(-val)));
			lv_add_reg_const("A",high(word(-val)));
			lv_add_reg_const("D",word(-val));
			return;
		}
		lv_free_reg_ABD();
}

void Bxx(char *cc, char *label);

void	NEG_V(char *v)
{
		char	*label = new_label();
		printf("\tNEG\t_%s+1\n",v);
		Bxx("NE",label);
		printf("\tDEC\t_%s\n",v);
		printf("%s\tCOM\t_%s\n",label,v);
		lv_free_var(v);
}

void	ABSD()
{
		char	*positive  = new_label();
		int		val;
		int		r = lv_search_reg_const("D",&val);
		TSTA();
		Bxx("PL",positive);
		NEGD();
		LABEL(positive);
		if(r){
			lv_add_reg_const("B",low(abs(val)));
			lv_add_reg_const("A",high(abs(val)));
			lv_add_reg_const("D",word(abs(val)));
			return;
		}
		lv_free_reg_ABD();
}


void	Trr(char *r1,char *r2)
{
		int		val;
		printf("\tT%s%s\n",r1,r2);
		if(lv_search_reg_const(r1,&val)){
			lv_add_reg_const(r2,val);
			lv_add_reg_const("D",make_word(val,val));
			return;
		}
		lv_free_reg("D");
		lv_free_reg(r1);
}
void	TAB()
{
		Trr("A","B");
}
void	TBA()
{
		Trr("B","A");
}
void	CLRr(char *r)
{
		if(lv_search_reg_const_val(r,0)){
//			printf("; CLR%s optimized\n",r);
//			do nothing
			return;
		}
		printf("\tCLR%s\n",r);
		lv_add_reg_const(r,0);
		char *pair = pair_reg(r);
		int val;
		if(lv_search_reg_const(pair,&val)){
			if(strcmp(r,"A")==0){
				lv_add_reg_const("D",val);
			}else{
				lv_add_reg_const("D",make_word(val,0));	// CLRBのときはAccA<<8がAccD
			}
		}
}
void	CLRA()
{
		CLRr("A");
}
void	CLRB()
{
		CLRr("B");
}
void	LDAr_I(char *r,int v)
{
		char	*pair = pair_reg(r);
		v = low(v);
		int		val;
		int		pval;
		int		exist;
		int		pexist;
		if((exist=lv_search_reg_const(r,&val))!=0
		&& (v==val)){									// rには既にその値が入っている
//			printf("; LDA%s #%d optimized\n",r,v);
//			do nothing
			return;
		}else if(v==0){									// LDAr_I 0ならCLRrで良い
			CLRr(r);									// 2 1
		}else if(exist && v==low(val+1)){				// 1少ないのでINCr
//			printf("; LDA%s #%d optimized to INC%s\n",r,v,r);
			INCr(r);									// 2 1
		}else if(exist && v==low(val-1)){				// 1多いのでDECr
//			printf("; LDA%s #%d optimized to DEC%s\n",r,v,r);
			DECr(r);									// 2 1
		}else if((pexist=lv_search_reg_const(pair,&pval))!=0// もう片方のAccの値は何？
		 	  &&  v==pval){								// そちらにあったのでTAB/TBAする
//			printf("; LDA%s #%d optimized to T%s%s\n",r,v,pair,r);
			Trr(pair,r);								// 2 1
		}else{
			// どこにもないのでLDArする
			printf("\tLDA%s\t#%d\n",r,v);				// 2 2
		}
		lv_add_reg_const(r,v);							// レジスタの値をアップデート
		if(pexist){										// ペアの値がわかってるなら、Dも設定できる
			if(strcmp(r,"A")==0){
				lv_add_reg_const("D",make_word(v,pval));
			}else{
				lv_add_reg_const("D",make_word(pval,v));
			}
		}	
}
void	LDAA_I(int v)
{
		LDAr_I("A",v);
}
void	LDAB_I(int v)
{
		LDAr_I("B",v);
}
void	LDAr_V0(char *r,char *str)
{
		printf("\tLDA%s\t_%s\n",r,str);
		lv_free_reg(r);
		lv_free_reg("D");
}
void	LDAA_V0(char *str)
{
		LDAr_V0("A",str);
}
void	LDAB_V0(char *str)
{
		LDAr_V0("B",str);
}
void	LDAr_V1(char *r,char *str)
{
		printf("\tLDA%s\t_%s+1\n",r,str);
		lv_free_reg(r);
		lv_free_reg("D");
}
void	LDAA_V1(char *str)
{
		LDAr_V1("A",str);
}
void	LDAB_V1(char *str)
{
		LDAr_V1("B",str);
}
void	LDAA_V(char *str)
{
		LDAA_V0(str);						// AとBで読む位置が異なる
}
void	LDAB_V(char *str)
{
		LDAB_V1(str);						// AとBで読む位置が異なる
}
void	LDAr_V(char *r,char *v)
{
		if(strcmp(r,"A")==0){
			LDAA_V0(v);
		}else if(strcmp(r,"B")==0){
			LDAB_V1(v);
		}else{
			error("; illegal register name %s\n",r);
		}
}
void	LDAr_X(char *r,int v)						// TODO: 名前はLDAr_X0 がいい？
{
		printf("\tLDA%s\t%d,X\n",r,v);
		lv_free_reg(r);
		lv_free_reg("D");
}
void	LDAA_X0(int v)
{
		LDAr_X("A",v);
}
void	LDAB_X0(int v)
{
		LDAr_X("B",v);
}
void	LDAB_X1(int v)
{
		LDAr_X("B",v+1);
}
void	LDAr_IVX(char *r,Node *node)
{
		if(isNUM(node)){
			if(strcmp(r,"A")==0){
				LDAA_I(high(node->val));
			}else if(strcmp(r,"B")==0){
				LDAB_I(low(node->val));
			}else{
				error("; LDAr_IVX: invalid register name '%s'\n",r);
			}
		}else if(isVAR(node)){
			LDAr_V(r,node->str);
		}else if(node->kind==ND_STACKTOP){
			LDAr_X(r,node->val);
		}else{
			printf("; LDA%s_IVX:",r);print_nodes_ln(node);
			error("; LDA%s_IVX ",r);
		}
}
void	LDAA_IVX(Node *node)
{
		LDAr_IVX("A",node);
}
void	LDAB_IVX(Node *node)
{
		LDAr_IVX("B",node);
}
void	LDD_I(int v)
{
		if(lv_search_reg_const_val("D",v)){
//			printf("; LDD #%d optimized\n",v);
//			do nothing
			return;
		}
		int	v1,v2;
		int b = lv_search_reg_const("B",&v2);
		int a = lv_search_reg_const("A",&v1);	// a,bレジスタの現在の値を調べる
		if(!(b && v2==low(v))){						// 下位バイトが一致しないなら、LOAD
			LDAB_I(low(v));
		}
		if(!(a && v1==high(v))){					// 上位バイトが一致しないなら、LOAD
			LDAA_I(high(v));
		}
}
void	LDD_IV(char *v)
{
		printf("\tLDAB\t#_%s\n", 	v);
		printf("\tLDAA\t#_%s/256\n",v);
		lv_free_reg_ABD();
}
void	LDD_X(int v)
{
		LDAB_X1(v);
		LDAA_X0(v);
		lv_free_reg_D();
}
void	LDD_V(char *v)
{
		if(lv_search_reg_var("D",v)){
//			printf("; LDD var %s optimized\n",v);
//			do nothing
			return;
		}
		LDAB_V1(v);
		LDAA_V0(v);
		lv_add_reg_var("D", v);
}
void	LDD_L(char *str)
{
		printf("\tLDAB\t%s+1\n", str);
		printf("\tLDAA\t%s\n", str);
		lv_free_reg_ABD();
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

void	STD_V(char *str)
{
		printf("\tSTAB\t_%s+1\n",	str);
		printf("\tSTAA\t_%s\n",		str);
		lv_free_var(str);
		lv_add_reg_var("D",str);
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
void	STAr_Xn(char *r,int offset,int n)
{
		printf("\tSTA%s\t%d,X\n",r,offset+n);
}
void	STAA_X0(int v)
{
		STAr_Xn("A",v,0);
}
void	STAA_X1(int v)
{
		STAr_Xn("A",v,1);
}
void	STAB_X0(int v)
{
		STAr_Xn("B",v,0);
}
void	STAB_X1(int v)
{
		STAr_Xn("B",v,1);
}
void	STAA_V0(char *str)
{
		printf("\tSTAA\t_%s\n",str);
		lv_free_var(str);
}
void	STAA_V1(char *str)
{
		printf("\tSTAA\t_%s+1\n",str);
		lv_free_var(str);
}
void	STAA_V(char *str)
{
		STAA_V0(str);
		lv_free_var(str);
}
void	STAB_V0(char *str)
{
		printf("\tSTAB\t_%s\n",str);
		lv_free_var(str);
}
void	STAB_V1(char *str)
{
		printf("\tSTAB\t_%s+1\n",str);
		lv_free_var(str);
}
void	STAB_V(char *str)
{
		STAB_V1(str);
}
void	STAB_L0(char *str)
{
		printf("\tSTAB\t%s\n",str);
}
void	ABA()
{
		printf("\tABA\n");
		lv_free_reg("A");
		lv_free_reg("D");
}
void	ADDB_I(int v)
{
		int v1,v2;
		printf("\tADDB\t#%d\n", low(v));
		if(lv_search_reg_const("B",&v1)){
            lv_add_reg_const("B",low(v+v1));
			if(lv_search_reg_const("A",&v2)){
				lv_add_reg_const("D",make_word(v2,low(v+v1)));
			}
			return;
		}
		lv_free_reg_B();
		lv_free_reg_D();
}
void	ADCB_I(int v)
{
		printf("\tADCB\t#%d\n", low(v));	// キャリーがあるので結果は計算できない
		lv_free_reg_B();
		lv_free_reg_D();
}
void	ADCA_I(int v)
{
		printf("\tADCA\t#%d\n", low(v));	// キャリーがあるので結果は計算できない
		lv_free_reg_A();
		lv_free_reg_D();
}
void	ADD_I(int v)
{
		int	v0,v2;
		printf("\tADDB\t#%d\n", low(v));
		printf("\tADCA\t#%d\n", high(v));
		if(lv_search_reg_const("D",&v0)){
			v0 += v;
			lv_add_reg_const("B",low(v0));
			lv_add_reg_const("A",high(v0));
			lv_add_reg_const("D",v0);
			return;
		}else if(lv_search_reg_const("B",&v2)){
			v2 += low(v);
			lv_add_reg_const("B",low(v2));
			lv_free_reg_A();
			lv_free_reg_D();
			return;
		}
		lv_free_reg_ABD();
}
void	ADD_IV(char *v)
{
		printf("\tADDB\t#_%s+1\n",	v);
		printf("\tADCA\t#_%s\n",	v);
		lv_free_reg_ABD();
}
void	ADD_V(char *v)
{
		printf("\tADDB\t_%s+1\n",	v);
		printf("\tADCA\t_%s\n",		v);
		lv_free_reg_ABD();
}
void	ADD_L(char *str)
{
		printf("\tADDB\t%s+1\n", str);
		printf("\tADCA\t%s\n", str);
		lv_free_reg_ABD();
}
void	ADDB_X0(int v)
{
		printf("\tADDB\t%d,X\n", v);
		lv_free_reg_B();
		lv_free_reg_D();
}
void	ADD_X(int v)
{
		printf("\tADDB\t%d,X\n", v+1);
		printf("\tADCA\t%d,X\n", v);
		lv_free_reg_ABD();
}
void	SUB_I(int v)
{
		printf("\tSUBB\t#%d\n", low(v));
		printf("\tSBCA\t#%d\n", high(v));
		lv_free_reg_ABD();
}
void	SUB_X(int v)
{
		printf("\tSUBB\t%d,X\n", v+1);
		printf("\tSBCA\t%d,X\n", v);
		lv_free_reg_ABD();
}
void	SUB_V(char *v)
{
		printf("\tSUBB\t_%s+1\n",	v);
		printf("\tSBCA\t_%s\n",		v);
		lv_free_reg_ABD();
}
void	SUB_L(char *v)
{
		printf("\tSUBB\t%s+1\n",	v);
		printf("\tSBCA\t%s\n",		v);
		lv_free_reg_ABD();
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
void	ANDr_I(char *r,int v)
{
		v = low(v);
		switch(v){
		case 0:	CLRr(r);					// 厳密にはCフラグの扱いが異なる
				return;
		case 0x00ff:						// do nothing
				return;
		default:
				break;
		}
		int	v1,v2;
		int e1,e2;		// exist?
		char *pair = pair_reg(r);
		printf("\tAND%s\t#%d\n",r,low(v));
		e2 = lv_search_reg_const(pair,&v2);
		if((e1=lv_search_reg_const(r,&v1))){
			lv_add_reg_const(r,low(v&v1));
			if(e2){
				if(strcmp(r,"A")==0){
					lv_add_reg_const("D",make_word(v1,v2));
				}else{
					lv_add_reg_const("D",make_word(v2,v1));
				}
			}
			return;
		}
		lv_free_reg(r);
}
//
void	ANDA_I(int v)
{
		ANDr_I("A",v);
}
void	ANDB_I(int v)
{
		ANDr_I("B",v);
}
void	AND_I(int v)
{
		ANDB_I(low(v));
		ANDA_I(high(v));
}
void	AND_X(int v)
{
		printf("\tANDB\t%d,X\n", v+1);
		printf("\tANDA\t%d,X\n", v);
		lv_free_reg_ABD();
}
void	ANDB_V1(char *v)
{
		printf("\tANDB\t_%s+1\n",	v);
		lv_free_reg_B();
		lv_free_reg_D();
}
void	AND_V(char *v)
{
		printf("\tANDB\t_%s+1\n",	v);
		printf("\tANDA\t_%s\n",		v);
		lv_free_reg_ABD();
}
void	AND_L(char *v)
{
		printf("\tANDB\t%s+1\n",	v);
		printf("\tANDA\t%s\n",		v);
		lv_free_reg_ABD();
}
//
void	OR_I(int v)
{
		printf("\tORAB\t#%d\n", low(v));
		printf("\tORAA\t#%d\n", high(v));
		lv_free_reg_ABD();
}
void	ORAB_X(int v)
{
		printf("\tORAB\t%d,X\n", v+1);	// XXX
		lv_free_reg_B();
		lv_free_reg_D();
}
void	OR_X(int v)
{
		printf("\tORAB\t%d,X\n", v+1);
		printf("\tORAA\t%d,X\n", v);
		lv_free_reg_ABD();
}
void	OR_V(char *v)
{
		printf("\tORAB\t_%s+1\n",	v);
		printf("\tORAA\t_%s\n",		v);
		lv_free_reg_ABD();
}
void	OR_L(char *v)
{
		printf("\tORAB\t%s+1\n",	v);
		printf("\tORAA\t%s\n",		v);
		lv_free_reg_ABD();
}
//
void	EOR_I(int v)
{
		printf("\tEORB\t#%d\n", low(v));
		printf("\tEORA\t#%d\n", high(v));
		lv_free_reg_ABD();
}
void	EOR_X(int v)
{
		printf("\tEORB\t%d,X\n", v+1);
		printf("\tEORA\t%d,X\n", v);
		lv_free_reg_ABD();
}
void	EOR_V(char *v)
{
		printf("\tEORB\t_%s+1\n",	v);
		printf("\tEORA\t_%s\n",		v);
		lv_free_reg_ABD();
}
void	EOR_L(char *v)
{
		printf("\tEORB\t%s+1\n",	v);
		printf("\tEORA\t%s\n",		v);
		lv_free_reg_ABD();
}
void	BITA_I(int v)
{
		printf("\tBITA\t%d\n",low(v));
}
void	BITB_I(int v)
{
		printf("\tBITB\t%d\n",low(v));
}

void	CLRD()
{
		CLRB();
		CLRA();
}
//
//		optimize しないCLRD
//
void	CLRD_noopt()
{
		printf("\tCLRB\n");
		printf("\tCLRA\n");
		lv_free_reg_ABD();
}
void	CLR_V1(char *v)
{
		if(lv_search_reg_const_val("B",0)){
			STAB_V1(v);
		}else if(lv_search_reg_const_val("A",0)){
			STAA_V1(v);
		}else{
			printf("\tCLR\t_%s+1\n", v);
		}
		lv_free_var(v);
}
void	CLR_V0(char *v)
{
		if(lv_search_reg_const_val("B",0)){
			STAB_V0(v);
		}else if(lv_search_reg_const_val("A",0)){
			STAA_V0(v);
		}else{
			printf("\tCLR\t_%s\n", v);
		}
		lv_free_var(v);
}
void	STX_V(char *v);

void	CLR_V(char *v)
{
		if(lv_search_reg_const_val("X",0)){
			STX_V(v);
		}else{
			CLR_V1(v);
			CLR_V0(v);
		}
}

void	STD_X(int v);

void	CLR_X(int v)
{
		if(lv_search_reg_const_val("D",0)){
//			printf("; CLR ,X #%d optimized to STD\n",v);
			STD_X(v);
			return;
		}
		if(lv_search_reg_const_val("B",0)){
//			printf("; CLR ,X #%d optimized to STAB\n",v);
			STAB_X1(v);					// 6 2
			STAB_X0(v);					// 6 2
			return;
		}
		if(lv_search_reg_const_val("A",0)){
//			printf("; CLR ,X #%d optimized to STAA\n",v);
			STAA_X1(v);					// 6 2
			STAA_X0(v);					// 6 2
			return;
		}
		printf("\tCLR\t%d,X\n",v+1);	// 7 2
		printf("\tCLR\t%d,X\n",v);		// 7 2
}
void	CLR1_X1(int v)
{
		if(lv_search_reg_const_val("A",0)){
			STAA_X1(v);
		}else if(lv_search_reg_const_val("B",0)){
			STAB_X1(v);
		}else{
			printf("\tCLR\t%d+1,X\n",v);
		}
}
void	CLR1_X0(int v)
{
		if(lv_search_reg_const_val("A",0)){
			STAA_X0(v);
		}else if(lv_search_reg_const_val("B",0)){
			STAB_X0(v);
		}else{
			printf("\tCLR\t%d,X\n",v);
		}
}
void	CLR1_X(int v)
{
		CLR1_X0(v);
}
void	CMPA_I(int v)
{
		printf("\tCMPA\t#%d\n",v);
}
void	CMPB_I(int v)
{
		printf("\tCMPB\t#%d\n",v);
}
void	CMPr_X1(char *r,int v)
{
		printf("\tCMP%s\t%d+1,X\n",r,v);
}
void	CMPr_X0(char *r,int v)
{
		printf("\tCMP%s\t%d,X\n",r,v);
}
void	CMPA_X(int v)
{
		CMPr_X0("A",v);
}
void	CMPB_X(int v)
{
		CMPr_X0("B",v);
}
void	CMPA_V0(char *str)
{
		printf("\tCMPA\t_%s\n",str);
}
void	CMPA_V1(char *str)
{
		printf("\tCMPA\t_%s+1\n",str);
}
void	CMPA_V(char *str)
{
		CMPA_V0(str);
}
void	CMPB_V0(char *str)
{
		printf("\tCMPB\t_%s\n",str);
}
void	CMPB_V1(char *str)
{
		printf("\tCMPB\t_%s+1\n",str);
}
void	CMPB_V(char *str)
{
		CMPB_V1(str);
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
void	LDX_I(int v)
{
		if(lv_search_reg_const_val("X",v)){
//			printf("; LDX #%d optimized\n",v);
//			do nothing
			return;
		}
		printf("\tLDX\t#%d\n",v);
		lv_free_reg_X();
		lv_add_reg_const("X",v);
}
void	LDX_X(int v)
{
		printf("\tLDX\t%d,X\n",v);
		lv_free_reg_X();
}
void	LDX_V_I(char *str)
{
//		printf("; LDX_V search var %s\n",str);fflush(stdout);
		if(lv_search_reg_var("X",str)){
//			printf("; LDX var %s optimized\n",str);
//			do nothing
			return;
		}
		printf("\tLDX\t#_%s\n",str);
		lv_free_reg_X();
}
void	LDX_V(char *str)
{
//		printf("; LDX_V search var %s\n",str);fflush(stdout);
		if(lv_search_reg_var("X",str)){
//			printf("; LDX var %s optimized\n",str);
//			do nothing
			return;
		}
		printf("\tLDX\t_%s\n",str);
		lv_free_reg_X();
		lv_add_reg_var("X",str);
}
void	LDX_L(char *str)
{
		printf("\tLDX\t%s\n",str);
		lv_free_reg_X();
}
void	LDX_IL(char *str)
{
		printf("\tLDX\t#%s\n",str);
		lv_free_reg_X();
}
void	STX_V(char *v)
{
		printf("\tSTX\t_%s\n",	v);
		lv_free_var(v);
		lv_add_reg_var("X",v);
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
//
//		LDX	Var±256,512...	を生成する
//
void	LDX_Vp_sub(char *str)
{
		if(lv_search_reg_var("X",str)){
//			printf("; LDX var %s optimized\n",str);
//			do nothing
			return;
		}
		printf("\tLDX\t_%s\n",str);
		// レジスタ設定はLDX_vpで行う
}
void	LDX_Vp(char *str,int n)
{
		int	offset;
		// Xは既にV+n の形になっているか？
		if(lv_search_reg_X_array(LOC_LARRAY1,str,"",n)){
//			printf("; LDX var %s+%d optimized\n",str,n);
//			do nothing
			return;
		}
		char *reg;
		if((reg=lv_search_reg_any_array(str,"",&offset))!=NULL){
			printf("; LDX_Vp(%s,%d) search\n",str,n);
			printf("; lv_search_any_array found, reg=%s\n",reg);
			printf("; lv_search_any_array(%s+%d,\"\",offset=%d)\n",str,n,offset);fflush(stdout);
			error("; debug");
		}
		if(abs(n)%256!=0){
			error("; error LDX_vp %s %d\n",str,n);
		}
		printf("; LDX_Vp %s, offset=%d not found\n",str,n);
		if(n>0){
			for(int i=0; i<n; i+=256) INC_V0(str);
			LDX_Vp_sub(str);
			for(int i=0; i<n; i+=256) DEC_V0(str);
		}else{
			for(int i=0; i<abs(n); i+=256) DEC_V0(str);
			LDX_Vp_sub(str);
			for(int i=0; i<abs(n); i+=256) INC_V0(str);
		}
		printf("; LDX_Vp %s, offset=%d loaded.\n",str,n);
		lv_free_reg_X();
		lv_add_reg_array("X",LOC_LARRAY1,str,"",n);
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
		char	*keepTMP[] = {
				"DIVIDE","DIVPOW2","RANDOM",
				"MULTIPLY","PRHEX4","PRHEX2","PRINTTAB",
				"PRINTL","PRINTR","PRINTCR","PRINTCR","INPUT","KBIN_SUB",
				"PRINTSTR","ASCIN","CURPOS","MUSIC"
		};
		printf("\tJSR\t%s\n",to);
		for (int i=0; i<(int)(sizeof(keepTMP)/sizeof(*keepTMP)); i++){
			if(strcmp(to,keepTMP[i])==0){
				lv_free_reg_ABDX();
				return;
			}
		}
		lv_free_all();
}
void	JSR_X(int v)
{
		printf("\tJSR\t%d,X\n",v);
		lv_free_all();
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
		lv_free_reg_ABDX();
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
		lv_free_reg_ABDX();
#else
		STX_L("ADXWK");
		ASLD();
		ADD_L("ADXWK");
		STD_L("ADXWK");
#endif
}
//		TFR	D,X
char	*TDX()
{
		// 12cycle 6bytes (with DP)
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_free_reg_X();
		LDX_V(tmp);
		return	tmp;
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
//	配列 node は単純な配列か?
//		単純な配列とは、アドレス計算にDregを必要としないもの
//			A(offset)		-128<=offset<255 なら単純な配列
//			A:offset)		-256<=offset<511
//			A(V+offset)		A+V*2が既に一時変数TMPにあり、offsetは0-127
//			A:V+offset)		A+Vが既に一時変数TMPにあり、offsetは0-255
//							TMPではなくXに既にある場合は、Xが壊れることを考えて0を返す
//
int is_simple_array(Node *node)
{
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; gen_array_address error\n");
	}
	int	offset1,offset2;
	loc_type_t loc_type;
	if(node->kind==ND_ARRAY1){
		offset1 = 256;
		offset2 = 512;
		loc_type = LOC_LARRAY1;
	}else{
		offset1 = 128;
		offset2 = 256;
		loc_type = LOC_LARRAY2;
	}
	if(isNUM(node->lhs)){					// 添字が定数
		int offset = node->lhs->val;
		return(offset>=-offset1 && offset<offset2);
	}else if(isVAR(node->lhs)){		// 添字が単純変数
		char	*v1 = node->str;
		char	*v2 = node->lhs->str;
		return(lv_search_tmp_array(loc_type,v1,v2,0)!=NULL); // TMPnに計算結果があった
	}else if((node->lhs->kind==ND_ADD)
		&&   isVAR(node->lhs->lhs) && isNUM(node->lhs->rhs)
		&&   (node->lhs->rhs->val>=0) && (node->lhs->rhs->val<offset1)){
		char	*v1 = node->str;
		char	*v2 = node->lhs->lhs->str;
		return(lv_search_tmp_array(loc_type,v1,v2,0)!=NULL); // TMPnに計算結果があった
	}
	return	0;
}
//
//	var:offset) または var(offset)のアドレスを得る方法を返す
//	戻り値は変数名か一時変数名(TMPx)
//	エラーの時はNULLを返す
//		ex. A:1)
//				offset = 1
//				return var A
//		ex. A(1)
//				offset =  2
//				return var A
//		ex. A(V+1)
//				Dreg = A+V*2 (実際に計算する)
//				STD TMP
//				loc = TMP
//				offset = 2
//				return	TMP;
//		ex.
//		save: 保存しないといけないレジスタ。Xは保存される
//				"A" or "B" or "D"
//
char *
gen_array_laddress(Node *node,char *save,int *offset)
{
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; gen_array_address error\n");
		return NULL;
	}
	int	offset1,offset2,scale,max_DEX;
	loc_type_t loc_type;
	char *loc_char;
	if(node->kind==ND_ARRAY1){
		offset1 = 256;
		offset2 = 512;
		scale   = 1;
		max_DEX = 6;
		loc_type = LOC_LARRAY1;
		loc_char = ":";
	}else{
		offset1 = 128;
		offset2 = 256;
		scale   = 2;
		max_DEX = 3;
		loc_type = LOC_LARRAY2;
		loc_char = "(";
	}
	printf("; gen_array_address:");print_nodes_ln(node);
	if(isNUM(node->lhs)){					// 添字が定数
		int val = node->lhs->val;
		if(val>=0 && val<offset1){	// 0..255 or 0..127
			*offset = val*scale;
			printf("; gen_array_address 1: return %d,",*offset);print_nodes_ln(node);
			return	node->str;
		}
	}else if(isVAR(node->lhs)){		// 添字が単純変数
		char	*v1 = node->str;
		char	*v2 = node->lhs->str;
		char	*tmp1;
		// TMPnに計算結果があった
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			*offset = 0;
			printf("; gen_array_address 2: return %d,%s\n",*offset,tmp1);
			return	tmp1;
		}
		gen_save(save);
		LDD_V(node->lhs->str);
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		ADD_V(node->str);			// 配列の添字はAccABにある
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_add_reg_array(tmp,loc_type,v1,v2,0);			// offsetは0で良い
		gen_restore(save);
		*offset = 0;
		printf("; gen_array_address 3: return %d,",*offset);print_nodes_ln(node);
		return	tmp;
	}else if((node->lhs->kind==ND_ADD)
		&&   isVAR(node->lhs->lhs) && isNUM(node->lhs->rhs)
		&&   (node->lhs->rhs->val>=0) && (node->lhs->rhs->val<offset1)){
		// V+var+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 str=V (+ (ND_VAR var) (ND_NUM offset)))
		char	*v1 = node->str;
		char	*v2 = node->lhs->lhs->str;
		Node	*expr = node->lhs->lhs;
		int		val = node->lhs->rhs->val;
		printf("; array type V%sexpr+offset) optimize\n",(node->kind==ND_ARRAY1)?":":"(");fflush(stdout);
		char	*tmp1;
		// TMPnに計算結果がある
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			*offset = val*scale;
			printf("; gen_array_address 4: return %d,%s\n",*offset,tmp1);
			return	tmp1;
		}
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		ADD_V(v1);					// 配列の添字はAccABにある
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		gen_restore(save);
		lv_add_reg_array(tmp,loc_type,v1,v2,0);			// offsetは0で良い
		*offset = val*scale;
		printf("; gen_array_address 5: return %d,%s\n",*offset,tmp);
		return	tmp;
	}else if((node->lhs->kind==ND_ADD
		&&    isNUM(node->lhs->rhs) && node->lhs->rhs->val>=0 && node->lhs->rhs->val<offset1)){
		// V+expr+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 str=V (+ (expr) (ND_NUM offset)))
		char *v1 = node->str;
		Node *expr = node->lhs->lhs;
		int	val = node->lhs->rhs->val;
		printf("; array type V%sexpr+offset) optimize\n",loc_char);fflush(stdout);
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		ADD_V(v1);					// 配列の添字はAccABにある
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_add_reg_array(tmp,LOC_LADRS,"","",0);			// offsetは0で良い
		gen_restore(save);
		*offset = val*scale;
		printf("; gen_array_address 6: return %d,%s\n",*offset,tmp);
		return	tmp;
	}
	// 添字が一般の式の場合
	gen_save(save);
	gen_expr(node->lhs);	// 添字の計算をして
	if(node->kind==ND_ARRAY2){
		ASLD();
	}
	ADD_V(node->str);		// 配列の添字はAccABにある
	char *tmp = lv_search_free_tmp();
	STD_V(tmp);
	lv_add_reg_array(tmp,LOC_LADRS,"","",0);			// offsetは0で良い
	gen_restore(save);
	*offset = 0;
	printf("; gen_array_address 7: return %d,%s\n",*offset,tmp);
	return tmp;
}
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
//		save: 保存しないといけないレジスタ。Xは壊れる前提
//				"A" or "B" or "D"
//
int
gen_array_address(Node *node,char *save)
{
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; gen_array_address error\n");
	}
	int	offset1,offset2,scale,max_DEX;
	loc_type_t loc_type;
	char *loc_char;
	if(node->kind==ND_ARRAY1){
		offset1 = 256;
		offset2 = 512;
		scale   = 1;
		max_DEX = 6;
		loc_type = LOC_LARRAY1;
		loc_char = ":";
	}else{
		offset1 = 128;
		offset2 = 256;
		scale   = 2;
		max_DEX = 3;
		loc_type = LOC_LARRAY2;
		loc_char = "(";
	}
//	printf("; gen_array_address:");print_nodes_ln(node);
	if(isNUM(node->lhs)){					// 添字が定数
		int offset = node->lhs->val;
		if(offset>=0 && offset<offset1){	// 0..255 or 0..127
			LDX_V(node->str);
			return offset*scale;
		}else if(offset<0 && offset>=-max_DEX){			// -1..-6 or -1..-3
			LDX_V(node->str);
			for(int i=0;i<abs(offset*scale);i++){
				DEX();
			}
			return	0;
		}else if(offset>=offset1 && offset<offset2){	// 256..511 or 128..255
			LDX_Vp(node->str,256);
			return	offset*scale-256;
		}else if(offset<0 && offset>=-offset1){			// -1..-256 or -1..-128
			LDX_Vp(node->str,-offset1);
			return	offset*scale+offset1;
		}
	}else if(isVAR(node->lhs)){		// 添字が単純変数
		char	*v1 = node->str;
		char	*v2 = node->lhs->str;
		if(lv_search_reg_X_array(loc_type,v1,v2,0)){	// 既にIXにある
//			printf("; LDX_V optimized. IX already have %s%s%s)\n",v1,loc_char,v2);
//			do nothing
			return	0;
		}
		char	*tmp1;
		// TMPnに計算結果があった
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			if(lv_search_reg_var("X",tmp1)){
//				printf("; LDX_V optimized. IX already have %s\n",tmp1);
//				//do nothing
			}else{
//				printf("; LDX_V optimized. %s has array %s%s%s)\n",tmp1,v1,loc_char,v2);
				LDX_V(tmp1);
			}
			return	0;
		}
		gen_save(save);
		LDD_V(node->lhs->str);
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		ADD_V(node->str);			// 配列の添字はAccABにある
		char *tmp2=TDX();						// TFR D,X
		lv_add_reg_array(tmp2,loc_type,node->str,node->lhs->str,0);
		lv_add_reg_array_nofree("X", loc_type,node->str,node->lhs->str,0);
		gen_restore(save);
		return 0;
	}else if((node->lhs->kind==ND_ADD)
		&&   isVAR(node->lhs->lhs) && isNUM(node->lhs->rhs)
		&&   (node->lhs->rhs->val>=0) && (node->lhs->rhs->val<offset1)){
		// V+var+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 str=V (+ (ND_VAR var) (ND_NUM offset)))
		char	*v1 = node->str;
		char	*v2 = node->lhs->lhs->str;
		Node	*expr = node->lhs->lhs;
		int		offset = node->lhs->rhs->val;
		printf("; array type V%sexpr+offset) optimize\n",(node->kind==ND_ARRAY1)?":":"(");fflush(stdout);
		char	*tmp1;
		// IXに既に計算結果がある
		if(lv_search_reg_X_array(loc_type,v1,v2,0)){	// 既にIXにある
//			printf("; LDX_V optimized. IX already have %s:%s)\n",v1,v2);
			return	offset*scale;
		}
		// TMPnに計算結果がある
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			if(lv_search_reg_var("X",tmp1)){
//				printf("; LDX_V optimized. IX already have %s\n",tmp1);
//				do nothing
			}else{
//				printf("; LDX_V optimized. %s has array %s:%s)\n",tmp1,v1,v2);
				LDX_V(tmp1);
			}
			return	offset*scale;
		}
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		ADD_V(v1);					// 配列の添字はAccABにある
		char *tmp2 = TDX();									// TFR D,X
		lv_add_reg_array(tmp2,loc_type,v1,v2,0);			// offsetは0で良い
		lv_add_reg_array_nofree("X", loc_type,v1,v2,0);
		gen_restore(save);
		return offset*scale;			// offsetは別に返す
	}else if((node->lhs->kind==ND_ADD
		&&    isNUM(node->lhs->rhs) && node->lhs->rhs->val>=0 && node->lhs->rhs->val<offset1)){
		// V+expr+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 str=V (+ (expr) (ND_NUM offset)))
		char *v1 = node->str;
		Node *expr = node->lhs->lhs;
		int	offset = node->lhs->rhs->val;
		printf("; array type V%sexpr+offset) optimize\n",loc_char);fflush(stdout);
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		ADD_V(v1);					// 配列の添字はAccABにある
		if(node->kind==ND_ARRAY2){
			ASLD();
		}
		char *tmp2 = TDX();			// TFR D,X
		gen_restore(save);
		return offset;				// offsetは別に返す
	}
	// 添字が一般の式の場合、TMPには保存しない、流用もできない
	gen_save(save);
	gen_expr(node->lhs);	// 添字の計算をして
	if(node->kind==ND_ARRAY2){
		ASLD();
	}
	ADD_V(node->str);		// 配列の添字はAccABにある
	TDX();					// TFR D,X
	gen_restore(save);
	return 0;
}

//
//	条件比較を見て短い分岐を作成する
//		条件成立しなかった時にlabelに飛ぶ
//
void
gen_skip_if_false(Node *node,char *if_false)
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
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(node->lhs)){
			if(lv_search_reg_var("D",node->lhs->str)){
				TSTA();
#if	EMU_CPU_BUGS
			}else if(lv_search_reg_var("X",node->lhs->str)){
				CPX_I(0);
#endif
			}else{
				TST_V0(node->lhs->str);	// 6cycle,3byte
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
		return;
	}else if(isARRAY1(node->lhs) && isNUM(node->rhs) && isEQorNE(node)
			&& node->rhs->val>=0 && node->rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_branch_if_true ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X0(offset);
		if(node->rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(node->rhs->val);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("NE",if_false);
		}else{						// lhs!=rhs
			Bxx("EQ",if_false);	
		}
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
		return;
	}else{
		gen_expr(node->lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STD_V(tmp);
		gen_expr(node->rhs);
		// 以下、AccAB(rhs)とStackTOP(lhs)を比較(rhs-lhsの値を見ている）
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_V1(tmp);
					Bxx("NE",if_false);
					CMPA_V0(tmp);
					Bxx("NE",if_false);
					break;
		case ND_NE:	CMPB_V1(tmp);
					Bxx("NE",if_true);
					CMPA_V0(tmp);
					Bxx("EQ",if_false);
					break;
		case ND_GE:	SUB_V(tmp);
					Bxx("GE",if_false);	// rhs-lhs<0 (lhs<rhs) then true
					break;
		case ND_GT:	SUB_V(tmp);
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("NE",if_false);
					break;
		case ND_LE:	SUB_V(tmp);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("EQ",if_false);
					break;
		case ND_LT:	SUB_V(tmp);
					Bxx("LT",if_false);
					break;
		default:
					error("Bxx not known\n");
		}
		lv_free_reg(tmp);
		LABEL(if_true);
		return;
	}
}
//
//	条件が成立したときはlabelに飛ぶ
//
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
	gen_skip_if_false(node,if_false);			// skipするので条件反転する
	JMP(label);
	LABEL(if_false);
}
//
//	条件比較を見て短い分岐を作成する
//		条件成立した時はlabelに飛ぶ
//
void
gen_skip_if_true(Node *node,char *if_true)
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
	// 0との比較はサボれる場合がある
	if(isNUM(node->rhs) && node->rhs->val==0
	&& (node->kind==ND_GE || node->kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(node->lhs)){
			if(lv_search_reg_var("D",node->lhs->str)){
				TSTA();
#ifdef	EMU_CPU_BUGS
			}else if(lv_search_reg_var("X",node->lhs->str)){
				CPX_I(0);
#endif
			}else{
				TST_V0(node->lhs->str);	// 6cycle,3byte
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
		return;
	}else if(isARRAY1(node->lhs) && isNUM(node->rhs) && isEQorNE(node)
			&& node->rhs->val>=0 && node->rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_branch_if_false ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(node->lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X0(offset);
		if(node->rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(node->rhs->val);
		}
		if(node->kind==ND_EQ){		// lhs==rhs?
			Bxx("EQ",if_true);
		}else{						// lhs!=rhs
			Bxx("NE",if_true);	
		}
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
		return;
	}else{
		gen_expr(node->lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STD_V(tmp);
		gen_expr(node->rhs);
		// 以下、AccAB(rhs)とStackTOP(lhs)を比較(rhs-lhsの値を見ている）
		switch(node->kind){ // if cc then jump to label otherwise jump to if_false
		case ND_EQ:	CMPB_V1(tmp);
					Bxx("NE",if_false);
					CMPA_V0(tmp);
					Bxx("EQ",if_true);
					break;
		case ND_NE:	CMPB_V1(tmp);
					Bxx("NE",if_true);
					CMPA_V0(tmp);
					Bxx("NE",if_true);
					break;
		case ND_LT:	//print_debug("ND_LT"); // lhs < rhs?   (ie. rhs-lhs>0)
					SUB_V(tmp);
					Bxx("GE",if_true);	// rhs-lhs<0 (lhs<rhs) then true
					break;
		case ND_LE:	SUB_V(tmp);
					Bxx("LT",if_false);
					Bxx("NE",if_true);
					TSTB();
					Bxx("NE",if_true);
					break;
		case ND_GT:	SUB_V(tmp);
					Bxx("LT",if_true);
					Bxx("NE",if_false);
					TSTB();
					Bxx("EQ",if_true);
					break;
		case ND_GE:	SUB_V(tmp);
					Bxx("LT",if_true);
					break;
		default:
					error("Bxx not known\n");
		}
		lv_free_reg(tmp);
		LABEL(if_false);
		return;
	}
}
//
//	条件が成立しないときはlabelに飛ぶ
//
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
	char	*if_true = new_label();
	gen_skip_if_true(node,if_true);			// skipするので条件反転する
	JMP(label);
	LABEL(if_true);
}

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
	char *if_false = new_label();
	gen_skip_if_false(node,if_false);
	LDAB_I(1);
	SKIP1();
	LABEL(if_false);
	CLRB();
	lv_free_reg_B();
	CLRA();
	return;
}

//
//	間接1バイト変数の計算。計算結果はAccBに置く。CLRA無し。
//
void gen_ARRAY1(Node *node)
{
	int offset = gen_array_address(node,"");
	LDAB_X0(offset);
	return;
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
			LDAB_X0(offset);
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
	case ND_CURSORADRS: {
			JSR("CURPOS");
			char *tmp = lv_search_free_tmp();
			STX_V(tmp);
			LDD_V(tmp);
			lv_free_reg(tmp);
			}
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
			lv_free_reg_B();
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
			&& lv_search_reg_var("D",node->rhs->str)){	// AccDに右項の変数がある
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
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STD_V(tmp);
				gen_expr(node->rhs);
				ADD_V(tmp);
				lv_free_reg(tmp);
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
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STD_V(tmp);
				printf("; ND_SUB lhs save to %s\n",tmp);
				gen_expr(node->rhs);
				NEGD();
				ADD_V(tmp);
				lv_free_reg(tmp);
				return;
			}
			break;
	case ND_MUL: {
			char *tmp;
			if(isVAR(node->lhs)&&isVAR(node->rhs)		// 単純変数同士の加算
			&& lv_search_reg_var("D",node->rhs->str)){	// AccDに右項の変数がある
				Node *old = node;						// 左右を入れ替える
				node = new_copy_node(old);
				node->lhs = old->rhs;
				node->rhs = old->lhs;
			}
			if(isCompare(node->lhs)){		// 比較演算結果との乗算 ex. (x>=y)*32
				char *label = new_label();
				char *label2 = new_label();
				if(isNUMorVAR(node->rhs)){	// 右項は数値か単純変数
					gen_skip_if_false(node->lhs,label);
					LDD_IVX(node->rhs);
					SKIP2();
					LABEL(label);
					CLRD_noopt();
				}else{						// 右は式
					gen_expr(node->lhs);
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					gen_expr(node->rhs);
					TST_V1(tmp);
					Bxx("NE",label);		// if 1, rhs is result.
					CLRD();					// otherwise 0
					LABEL(label);
					lv_free_reg_X();
					lv_free_reg(tmp);
				}
				return;
			}else if(isNUM(node->rhs)){
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
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					ASLD_N(2);
					ADD_V(tmp);
					if(node->rhs->val==20){
						ASLD();
						ASLD();
					}else if(node->rhs->val==10){
						ASLD();
					}
					lv_free_reg(tmp);
					return;
				case 12:
				case 6:
				case 3:
					gen_expr(node->lhs);
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					ASLD_N(1);
					ADD_V(tmp);
					if(node->rhs->val==12){
						ASLD();
						ASLD();
					}else if(node->rhs->val==6){
						ASLD();
					}
					lv_free_reg(tmp);
					return;
				case 18:
				case 9:
					gen_expr(node->lhs);
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					ASLD_N(3);
					ADD_V(tmp);
					if(node->rhs->val==18){
						ASLD();
					}
					lv_free_reg(tmp);
					return;
				case 14:
				case 7:
					gen_expr(node->lhs);
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					ASLD_N(3);
					SUB_V(tmp);
					if(node->rhs->val==14){
						ASLD();
					}
					lv_free_reg(tmp);
					return;
				case 17:
				case 15:
					gen_expr(node->lhs);
					tmp = lv_search_free_tmp();
					STD_V(tmp);
					ASLD_N(4);
					if(node->rhs->val==15){
						SUB_V(tmp);
					}else{
						ADD_V(tmp);
					}
					lv_free_reg(tmp);
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
			tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STD_V(tmp);
			gen_expr(node->rhs);
			LDX_V_I(tmp);
			JSR("MULTIPLY");
			lv_free_reg(tmp);
			}
			break;
	case ND_DIV: {
			// 余り(MOD)の処理があるので簡単ではない
			// 符号拡張に注意
			if(isNUM(node->lhs) && isNUM(node->rhs)){	// 定数同士の除算の場合、大抵はMODの設定
				int16_t	x=node->lhs->val;
				int16_t	y=node->rhs->val;
				int16_t	d=x/y;
				int16_t m=abs(x%y);						// GAME68では余りは常に正
				if(lv_is_free("X")){
					LDX_I(m);
					STX_V("MOD");
				}else{
					LDD_I(m);
					STD_V("MOD");
				}
				LDD_I(d);
				return;
			}
			if(isSameVAR(node->lhs,node->rhs)){			// 同じ変数の割り算 K/K など
				if(lv_is_free("X")){
					LDX_I(0);
					STX_V("MOD");
				}else{
					LDD_I(0);
					STD_V("MOD");
				}
				LDD_I(1);
				return;
			}
			if(isNUM(node->rhs) && node->rhs->val==2){	// 2で割る時は特殊処理
//				printf("; DIV /2 debug: ");print_nodes_ln(node);
				if(isVAR(node->lhs)){		// 単純変数/2
					LDAB_V1(node->lhs->str);// 4 3
					ANDB_I(1);				// 2 2
					STAB_V1("MOD");			// 5 3
					CLR_V0("MOD");			// 6 3	↑16 11
					gen_expr(node->lhs);
				}else{						// expr/2
					gen_expr(node->lhs);
					PSHB();					// 4 1
					ANDB_I(1);				// 2 2
					STAB_V1("MOD");			// 5 3
					CLR_V0("MOD");			// 6 3
					PULB();					// 4 1	↑21 10
				}
				ASRD();
				ADCB_I(0);
				ADCA_I(0);
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
			char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STD_V(tmp);
			gen_expr(node->rhs);
			LDX_V_I(tmp);
			JSR("DIVIDE");
			lv_free_reg(tmp);
			}
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
			}
			break;
	case ND_RELMUL: {  //  関係演算同士の*
			gen_expr(node->lhs);
			char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STAB_V1(tmp);
			gen_expr(node->rhs);
			ANDB_V1(tmp);
			CLRA();
			lv_free_reg(tmp);
			}
			break;
	case ND_RELADD: { //  関係演算同士の+ は普通の加算で良い
			gen_expr(node->lhs);
			char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STAB_V1(tmp);
			gen_expr(node->rhs);
			ANDB_V1(tmp);							// 256回足したら死ぬかも
			CLRA();
			lv_free_reg(tmp);
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
			lv_free_all();	// レジスタ割り当て全てクリア
		}
		current = node;
		dump_loc_table();
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
			lv_free_all();	// レジスタ割り当て全てクリア
		}
		return;
	case ND_ASSIGN: {
			Node *lhs = node->lhs;		// 代入先変数
			Node *rhs = node->rhs;		// 右辺
//			printf("; debug ND_ASSIGN ");print_nodes_ln(node);
			if(node->lhs->kind==ND_VAR){		// ND_SETVARになってるはずだが…
				printf("; debug ND_SETVAR?? ");print_nodes_ln(node);
				gen_expr(rhs);
				gen_store_var(lhs);
			}else if(lhs->kind==ND_ARRAY1){
//				printf("; debug ND_ASSIGN to ND_ARRAY1:");print_nodes_ln(node);
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					CLR1_X(offset);
					return;
				}else if(isNUMorVAR(rhs)){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					LDAB_IVX(rhs);
					STAB_X0(offset);
					return;
				}
#if	1
				if(!has_side_effect(lhs)){
//					printf("; ND_ASSIGN lhs no side effect");print_nodes_ln(lhs);
					gen_expr(rhs);
					int offset = gen_array_address(lhs,"B");
					STAB_X0(offset);
					return;
				}
#endif
				gen_expr(lhs->lhs);						// calculate subscript
				ADD_V(lhs->str);						// 左辺のアドレスはDにある
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STD_V(tmp);
				gen_expr(rhs);
				LDX_V(tmp);
				STAB_X0(0);
				lv_free_reg(tmp);
				return;
			}else if(lhs->kind==ND_ARRAY2){
				//  (ND_ASSIGN (ND_ARRAY2 str=N (ND_VAR str=I)) (ND_NUM 0))
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					if((!lv_search_reg_const_val("A",0))
					&& (!lv_search_reg_const_val("B",0))){
						CLRA();
					}
					CLR_X(offset);
					return;
				}else if(isNUM(rhs) && (high(rhs->val)==low(rhs->val))){	// -1,257など
					int val=low(rhs->val);
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					if(lv_search_reg_const_val("B",val)){
						STAB_X1(offset);
						STAB_X0(offset);
					}else if(lv_search_reg_const_val("A",val)){
						STAA_X1(offset);
						STAA_X0(offset);
					}else{					// Xが使えないので、こちらが速い
						LDAB_I(val);
						STAB_X1(offset);
						STAB_X0(offset);
					}
					return;
				}else if(isNUMorVAR(rhs)){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					LDD_IVX(rhs);
					STD_X(offset);
					return;
				}
#if	0
				printf("; has_side_effetct? %d: ",has_side_effect(lhs));print_nodes_ln(lhs);
				if((!has_side_effect(lhs))
				&& (is_simple_array(lhs))){
					gen_expr(rhs);
					int offset = gen_array_address(lhs,"D");
					STD_X(offset);
					return;
				}
#endif
#if	1
				// debug
				{
				int  offset;
				char *loc = gen_array_laddress(lhs,"",&offset);
				dump_loc_table();
				printf("; offset = %d,loc=%s\n",offset,loc);
				gen_expr(rhs);
				LDX_V(loc);
				STD_X(offset);
				}
#else
				int	offset = gen_array_address(lhs,"");	// Xにアドレスが入っている
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STX_V(tmp);
				gen_expr(rhs);
				LDX_V(tmp);
				STD_X(offset);
				lv_free_tmp(tmp);
#endif
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
			int v;
			if(lv_search_reg_const("A",&v)){
				if(v){							// Aが非0(あるの?)
					return;						// 常に正なのでIFが成立してしまう
				}
				// A==0である
				TSTB();
				Bxx("NE",IF_TRUE);
				JMP(NEXT_LINE);
				LABEL(IF_TRUE);
				return;
			}
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
		lv_free_all();	// レジスタ割り当て全てクリア
		return;
	case ND_GOTO:
		if(node->lhs->kind!=ND_NUM){
			error("GOTO linenumber not constant\t");
			return;
		}
		JMP(new_line_label(node->val));
		//purge_loc_all();	// レジスタ割り当てクリア
		return;
	case ND_GOSUB:
		if(node->lhs->kind!=ND_NUM){
			error("GOSUB linenumber not constant\t");
			return;
		}
		JSR(new_line_label(node->val));
		lv_free_all();	// レジスタ割り当て全てクリア
		return;
	case ND_DO:
		LABEL(new_do_label(node->val));
		lv_free_all();	// レジスタ割り当て全てクリア
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
			// 終値はoptimizerで+1されており、ND_NEXTでの比較はGEで行える
			char	*FOR_LABEL=new_for_label(node->val);
			char	*TO_LABEL=new_to_label(node->val);
			FORTO[node->val].type  = node->rhs->kind;
			FORTO[node->val].var   = node->str;
			FORTO[node->val].loop  = FOR_LABEL;
			FORTO[node->val].to    = TO_LABEL;
			gen_stmt(node->lhs);
			printf("; ND_FOR %s opt=%d\n",node->str,ofl[node->val].opt);
			if(ofl[node->val].opt){	// control var+arrayの初期化
				printf("; ND_FOR set control var %s\n",node->str);
				for(int i=0; i<ofl[node->val].n; i++){	// 使っているARRAYについて
					printf("; ND_FOR %s(%s)\n",ofl[node->val].arrays[i],node->str);
					int	scale = (ofl[node->val].type[i]==ND_ARRAY1)?1:2;
					Node *new = new_unary(ND_SETVAR,
									new_binary(ND_ADD,
										new_node_var(ofl[node->val].arrays[i]),
										new_binary(ND_MUL, node->lhs->lhs, new_node_num(scale))));
					new->str = ofl[node->val].label[i];
					printf(";   ");print_nodes_ln(new);fflush(stdout);
					new = node_opt(new);
					printf("; =>");print_nodes_ln(new);fflush(stdout);
					gen_stmt(new);
				}
			}
			printf("; ND_FOR end of conttol var %s\n",node->str);
//			dump_loc_table();
			// 終値は+1されている。数値のときはImmediateにするために、値を覚えておく
			if(isNUM(node->rhs)){
				FORTO[node->val].val = node->rhs->val;
			}else{	// その他は素直に代入文生成
				Node *to_node = new_unary(ND_SETVAR,node->rhs);
				to_node->str = TO_LABEL;
//				printf("; node:");print_nodes_ln(node);fflush(stdout);
//				printf("; to_node:");print_nodes_ln(to_node);fflush(stdout);
//				printf("; ");print_nodes_ln(to_node);fflush(stdout);
				gen_stmt(to_node);
			}
			LABEL(FOR_LABEL);	// NEXTから戻ってくる場所
			lv_free_all();	// レジスタ割り当て全てクリア
		}
		return;
	case ND_NEXT: {
		// (ND_NEXT J (+ (ND_VAR J) 1 ))
		char	*FOR_LABEL=FORTO[node->val].loop;
		char	*TO_LABEL =FORTO[node->val].to;
		char	*NEXT_LABEL=new_label();
		printf("; ND_NEXT ");print_nodes_ln(node);
		printf("; optimize? %d\n",ofl[node->val].opt);
		printf("; node->lhs->lhs:");print_nodes_ln(node->lhs->lhs);
		if(ofl[node->val].opt
		&& node->lhs->kind==ND_ADD
		&& strcmp(node->lhs->lhs->str,ofl[node->val].var)==0
		&& isNUM(node->lhs->rhs) 
		&& ((node->lhs->rhs->val==1)||(node->lhs->rhs->val==2))){	// control var+arrayの増分処理
			printf("; ND_NEXT control var %s increment\n",node->str);
			for(int i=0; i<ofl[node->val].n; i++){	// 使っているARRAYについて
				char *var = ofl[node->val].label[i];
				printf("; ND_NEXT pseudo var %s increment\n",var);
				int scale = (ofl[node->val].type[i]==ND_ARRAY1)?1:2;
				Node *new = new_unary(ND_SETVAR,new_binary(ND_ADD,new_node_var(var),
												new_binary(ND_MUL,	node->lhs->rhs,
																	new_node_num(scale))));
				new->str = var;
				printf("; ND_NEXT :");print_nodes_ln(new);
				new = node_opt(new);
				printf(";      => :");print_nodes_ln(new);
				gen_stmt(new);
			}
		}
		gen_expr(node->lhs);		// LDX,INC,STX 5+4+6=15よりもLDD/ADD/STD 8+4+10=22が良い
		STD_V(node->str);			// 次のLDDが不要になるので
//		LDD_V(node->str);
		if(FORTO[node->val].type==ND_NUM){
			printf("; FORTO[node->val].val=%d\n",FORTO[node->val].val);
			SUB_I(FORTO[node->val].val);
		}else{
			SUB_V(TO_LABEL);
		}
		Bxx("PL",NEXT_LABEL);
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
		gen_expr(node->lhs);
		STAB_L0("WPRINTR");
		gen_expr(node->rhs);
		JSR("PRINTR");
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
	case ND_SETVAR: {					// 左辺は単純変数
			if(isNUM(node->lhs)){		// 右辺が数値
				int	v = node->lhs->val;
				if(lv_search_reg_const_val("X",v)){				// IXにあるか?
					STX_V(node->str);							// 6 3
				}else if(lv_search_reg_const_val("D",v)){		// Dにあるか
					STD_V(node->str);							// 10 6	// LDX_Iが良い?
				}else{
					LDX_I(node->lhs->val);						// 3 3
					STX_V(node->str);							// 6 3
				}
			}else if(isVAR(node->lhs)){	// 右辺が単純変数
				char	*v = node->lhs->str;
				if(lv_search_reg_var("X",v)){					// IXにあるか?
					STX_V(node->str);							// 6 3
				}else if(lv_search_reg_var("D",v)){				// Dにあるか
					STD_V(node->str);							// 10 6
				}else{
					LDX_V(node->lhs->str);						// 5 3
					STX_V(node->str);							// 6 3
				}
			}else if(isARRAY(node->lhs)){						// Z=A(I) or Z=A:I)
//				printf("; ND_SETVAR ");print_nodes_ln(node);
				int offset = gen_array_address(node->lhs,"");	// IXにA(I) or A:I)のアドレスが入る
				if(node->lhs->kind==ND_ARRAY1){
					LDAB_X0(offset);
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
				STAB_X0(lhs->lhs->val);
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
			lv_free_all();
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
	case ND_INCVAR: {
//				printf("; ND_INCVAR %s\n",node->str);
				char *v = node->str;
				if(lv_search_reg_var("X",v)){					// IXにあるか?
					INX();
					STX_V(v);
				}else if(lv_search_reg_var("D",v)){				// Dにあるか
					ADD_I(1);
					STD_V(v);
				}else{
					LDX_V(v);
					INX();
					STX_V(v);
				}
			}
			return;
	case ND_DECVAR: {
//				printf("; ND_DECVAR %s\n",node->str);
				char *v = node->str;
				if(lv_search_reg_var("X",v)){					// IXにあるか?
					DEX();
					STX_V(v);
				}else if(lv_search_reg_var("D",v)){				// Dにあるか
					ADD_I(-1);
					STD_V(v);
				}else{
					LDX_V(v);
					DEX();
					STX_V(v);
				}
			}
			return;
	case ND_INC2VAR: {
//				printf("; ND_INC2VAR %s\n",node->str);
				char *v = node->str;
				if(lv_search_reg_var("X",v)){					// IXにあるか?
					INX();
					INX();
					STX_V(v);
				}else if(lv_search_reg_var("D",v)){				// Dにあるか
					ADD_I(2);
					STD_V(v);
				}else{
					LDX_V(v);
					INX();
					INX();
					STX_V(v);
				}
			}
			return;
	case ND_DEC2VAR: {
//				printf("; ND_INC2VAR %s\n",node->str);
				char *v = node->str;
				if(lv_search_reg_var("X",v)){					// IXにあるか?
					DEX();
					DEX();
					STX_V(v);
				}else if(lv_search_reg_var("D",v)){				// Dにあるか
					ADD_I(-2);
					STD_V(v);
				}else{
					LDX_V(v);
					DEX();
					DEX();
					STX_V(v);
				}
			}
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
	lv_free_all();
	printf("*\n");
	printf("* generated by GAME-CC compiler\n");
	printf("*   from %s\n",source_file_name);
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
	for(int i=1; i<=for_count; i++){
		if(FORTO[i].to){
			char	comment[256];
			sprintf(comment,"%s: FOR %s",FORTO[i].loop,FORTO[i].var);
			char	label[256];
			sprintf(label,"_%s",FORTO[i].to);
			RMB_comment(label,2,comment);
		}
	}
	for(int i=1; i<ofl_n+1; i++){
		if(ofl[i].opt==0){
			continue;
		}
		for(int j=0; j<ofl[i].n; j++){
			printf("_%s\tRMB\t2\t; for loop pseudo array\n",ofl[i].label[j]);
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
