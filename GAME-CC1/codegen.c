#include	"common.h"


int	string_count;
int	const_count=0;
int	const_table[1024];	// 定数テーブル
char *const_table_s[1024];	// 定数テーブル

char *add_const_table(int v)
{
	char	*s=calloc(1,16);
	if(v<0){
		v+=65536;
	}
	for(int i=0; i<const_count; i++){
		if(const_table[i]==v){
			return const_table_s[i];
		}
	}
	const_table[const_count]=v;
	sprintf(s,"C_%d",v);
	const_table_s[const_count]=s;
	return	const_table_s[const_count++];
}


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
			case LOC_USED:	printf("LOC_USED   : %s used for tempoary save\n",p->reg);
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
			case LOC_LARRAY1:printf("LOC_LARRAY1: reg %s, var %s:%s%s%d)\n",
												p->reg,p->var1,p->var2,(p->val>=0)?"+":"",p->val);
							break;
			case LOC_LARRAY2:printf("LOC_LARRAY2: reg %s, var:%s(%s%s%d)\n",
												p->reg,p->var1,p->var2,(p->val>=0)?"+":"",p->val);
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

void	lv_free_tmp(char *v);

//
//	 変数vの情報を消す
//
//		Storeした先の変数がArray(Var)のVarであり、かつXがArray(Var)のアドレスを持っている場合、
//		TMP0=&A(Var)
//		X=TMP0
//		STD_V(Var)
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
			if(strncmp(p->reg,"TMP",3)==0){
				lv_free_tmp(p->reg);
			}
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
//	 一時変数TMPxがLOC_LADRSなら消す
//
void	lv_free_tmp_ladrs(char *v)
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if(p->kind==LOC_LADRS && strcmp(p->reg,v)==0){
			lv_free_tmp(v);
		}
	}
//	printf("; lv_free_var end\n");fflush(stdout);
}
//
//	一時変数(LOC_USED,LOC_LADRS)の情報を消す
//
void	lv_free_used()
{
	for(int i=0; i<loc_table_size; i++){
		loc_table_t *p = &loc_table[i];
		if((p->kind==LOC_USED)||(p->kind==LOC_LADRS)){
			lv_free_tmp(p->reg);
		}
	}
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
//	printf("; lv_add_reg_array_nofree %s=%s,%s,%d\n",r,var1,var2,offset);fflush(stdout);
	if((kind!=LOC_LARRAY1)&&(kind!=LOC_LARRAY2)&&(kind!=LOC_LADRS)){
		error("; lv_add_reg_array_nofree: illegal kind %d\n",kind);fflush(stdout);
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
	error("; ld_add_array_nofree can't add to register A,B,D \n");
}
//
//	レジスタrに配列変数情報を追加する。既にrに情報があれば消してから追加する
//
void	lv_add_reg_array(char *r, loc_type_t kind, char *var1, char *var2, int offset)
{
//	printf("; lv_add_reg_array %s=%s,%s,%d\n",r,var1,var2,offset);fflush(stdout);
	if((kind!=LOC_LARRAY1)&&(kind!=LOC_LARRAY2)&&(kind!=LOC_LADRS)){
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
void	TST_X0(int v)
{
		printf("\tTST\t%d,X\n",v);
}
void	TST_X1(int v)
{
		printf("\tTST\t%d,X\n",v+1);
}
void	TST_X(int v)
{
		TST_X0(v);
}

void	LDAA_X0(int v);
void	LDAB_X0(int v);

void	TSTany_X0(int v)
{
		if(lv_is_free("B")){
			LDAB_X0(v);
		}else if(lv_is_free("A")){
			LDAA_X0(v);
		}else{
			TST_X0(v);
		}
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

void	LDAB_V0(char *str);
void	LDAA_V0(char *str);

void	TSTany_V0(char *str)
{
		if(lv_is_free("B")){
			LDAB_V0(str);
		}else if(lv_is_free("A")){
			LDAA_V0(str);
		}else{
			TST_V0(str);
		}
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
void	INXDEX(int n)
{
		if(n>0){
			while(n-->0){
				INX();
			}
		}else{
			while(n++<0){
				DEX();
			}
		}
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
		if((e=lv_search_reg_const("D",&val)) && val==0){
//			printf("; ASLD Dreg=%d\n",val);
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

void
Bxx(char *cc, char *label)
{
	printf("\tB%s\t%s\n",cc,label);
}
void
BRA(char *label)
{
	Bxx("RA",label);
}
void
BCC(char *label)
{
	Bxx("CC",label);
}
void
BCS(char *label)
{
	Bxx("CS",label);
}
void
BEQ(char *label)
{
	Bxx("EQ",label);
}
void
BGE(char *label)
{
	Bxx("GE",label);
}
void
BGT(char *label)
{
	Bxx("GT",label);
}
void
BHI(char *label)
{
	Bxx("HI",label);
}
void
BLE(char *label)
{
	Bxx("LE",label);
}
void
BLS(char *label)
{
	Bxx("LS",label);
}
void
BLT(char *label)
{
	Bxx("LT",label);
}
void
BMI(char *label)
{
	Bxx("MI",label);
}
void
BNE(char *label)
{
	Bxx("NE",label);
}
void
BVC(char *label)
{
	Bxx("VC",label);
}
void
BVS(char *label)
{
	Bxx("VS",label);
}
void
BPL(char *label)
{
	Bxx("PL",label);
}

void	NEG_V(char *v)
{
		char	*label = new_label();
		printf("\tNEG\t_%s+1\n",v);
		BNE(label);
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
		BPL(positive);
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
		lv_free_reg("D");
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
		int		exist=0;
		int		pexist=0;
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
void	LDAr_X0(char *r,int v)
{
		printf("\tLDA%s\t%d,X\n",r,v);
}
void	LDAr_X1(char *r,int v)
{
		printf("\tLDA%s\t%d,X\n",r,v+1);
}
void	LDAA_X0(int v)
{
		LDAr_X0("A",v);
}
void	LDAB_X0(int v)
{
		LDAr_X0("B",v);
}
void	LDAB_X1(int v)
{
		LDAr_X1("B",v);
}
void	LDAr_X(char *r,int v)						// TODO: 名前はLDAr_X0 がいい？
{
		if(strcmp(r,"A")==0){
			LDAA_X0(v);
		}else if(strcmp(r,"B")==0){
			LDAB_X1(v);
		}else{
			error("; illegal register name %s\n",r);
		}
		lv_free_reg(r);
		lv_free_reg("D");
}
void	LDAr_IVX(char *r,Node *node,int offset)
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
		}else if(isARRAY2(node)){
			LDAr_X(r,offset);
		}else{
			printf("; LDA%s_IVX:",r);print_nodes_ln(node);
			error("; LDA%s_IVX ",r);
		}
}
void	LDAA_IVX(Node *node,int offset)
{
		LDAr_IVX("A",node,offset);
}
void	LDAB_IVX(Node *node,int offset)
{
		LDAr_IVX("B",node,offset);
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
//		printf("; LDD_X %d\n",v);
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
void	ADDB_V1(char *v)
{
		int v1,v2;
		printf("\tADDB\t_%s+1\n", v);
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
void	SUB_IVX(Node *node,int offset)		// SUB # or Var or n,X
{
		if(isNUM(node)){
			SUB_I(node->val);
		}else if(isVAR(node)){
			SUB_V(node->str);
		}else if(isARRAY2(node)){
			SUB_X(offset);
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
void	CLR_X1(int v)
{
		if(lv_search_reg_const_val("A",0)){
			STAA_X1(v);
		}else if(lv_search_reg_const_val("B",0)){
			STAB_X1(v);
		}else{
			printf("\tCLR\t%d+1,X\n",v);
		}
}
void	CLR_X0(int v)
{
		if(lv_search_reg_const_val("A",0)){
			STAA_X0(v);
		}else if(lv_search_reg_const_val("B",0)){
			STAB_X0(v);
		}else{
			printf("\tCLR\t%d,X\n",v);
		}
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
void	CMPA_X0(int v)
{
		CMPr_X0("A",v);
}
void	CMPB_X(int v)
{
		CMPr_X1("B",v);
}
void	CMPB_X0(int v)
{
		CMPr_X0("B",v);
}
void	CMPB_X1(int v)
{
		CMPr_X1("B",v);
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
void	CMPA_IVX(Node *node,int offset)
{
		if(isNUM(node)){
			CMPA_I(high(node->val));
		}else if(isVAR(node)){
			CMPA_V(node->str);
		}else if(isARRAY2(node)){
			CMPA_X0(offset);
		}else{
			printf("; CMPA_IVX:");print_nodes_ln(node);
			error("; CMPA_IVX ");
		}
}
void	CMPB_IVX(Node *node,int offset)
{
		if(isNUM(node)){
			CMPB_I(low(node->val));
		}else if(isVAR(node)){
			CMPB_V(node->str);
		}else if(isARRAY2(node)){
			CMPB_X1(offset);
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
void	LDX_IVX(Node *node,int offset)
{
		if(isNUM(node)){
			LDX_I(node->val);
		}else if(isVAR(node)){
			LDX_V(node->str);
		}else if(isARRAY2(node)){
			LDX_X(offset);
		}else{
			printf("; LDX_IVX:");print_nodes_ln(node);
			error("; LDX_IVX ");
		}
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
void	CPX_IV(Node *node)
{
		if(isNUM(node)){
			CPX_I(node->val);
		}else if(isVAR(node)){
			CPX_V(node->str);
		}else{
			printf("; CPX_IV:");print_nodes_ln(node);
			error("; CPX_IV ");
		}
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
//		printf("; LDX_Vp %s, offset=%d not found\n",str,n);
		if(n>0){
			for(int i=0; i<n; i+=256) INC_V0(str);
			LDX_Vp_sub(str);
			for(int i=0; i<n; i+=256) DEC_V0(str);
		}else{
			for(int i=0; i<abs(n); i+=256) DEC_V0(str);
			LDX_Vp_sub(str);
			for(int i=0; i<abs(n); i+=256) INC_V0(str);
		}
//		printf("; LDX_Vp %s, offset=%d loaded.\n",str,n);
		lv_free_reg_X();
		lv_add_reg_array("X",LOC_LARRAY1,str,"",n);
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


void	RMB_comment(char *label,int size,char *comment)
{
		printf("%s\tRMB\t\%d\t; %s\n",label,size,comment);
}
void	RMB(char *label,int size)
{
		printf("%s\tRMB\t\%d\n",label,size);
}
void	FDB_V(char *label,int value)
{
		printf("_%s\tFDB\t\%d\n",label,value);
}
void	FDB(char *label,int value)
{
		printf("%s\tFDB\t\%d\n",label,value);
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
//	printf("; is_simple_array:");print_nodes_ln(node);
	if(node==NULL || !isARRAY(node)){
		printf("; not a array. why? ");print_nodes_ln(node);
		error("; is_simple_array error\n");
	}
	int	offset1,offset2;
	loc_type_t loc_type;
	if(isARRAY1(node)){
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
		error("; gen_array_laddress error\n");
		return NULL;
	}
	int	offset1,offset2,scale;
	loc_type_t loc_type;
	char *loc_char;
	if(isARRAY1(node)){
		offset1 = 256;
		offset2 = 512;
		scale   = 1;
		loc_type = LOC_LARRAY1;
		loc_char = ":";
	}else{
		offset1 = 128;
		offset2 = 256;
		scale   = 2;
		loc_type = LOC_LARRAY2;
		loc_char = "(";
	}
//	printf("; gen_array_laddress:");print_nodes_ln(node);
	if(isNUM(node->lhs)){					// 添字が定数
		int val = node->lhs->val;
		if(val>=0 && val<offset1){	// 0..255 or 0..127
			*offset = val*scale;
//			printf("; gen_array_laddress 1: return %d,",*offset);print_nodes_ln(node);
			return	node->str;
		}
		// 大きな定数 or 負の定数の場合
		// 256単位でアクセスすると使い回ししやすい
		int16_t	base = (val*scale)&0x0ff00;
		int16_t	off  = (val*scale)&0x000ff;
		// TMPnに計算結果があった
		char *tmp1;
		if((tmp1=lv_search_tmp_array(loc_type,node->str,"",base))!=NULL){
			*offset = off;
//			printf("; gen_array_laddress 1.4: return %d,%s\n",*offset,tmp1);
			return	tmp1;
		}
		gen_save(save);
		LDD_I(base);
		ADD_V(node->str);				//	AccAB = address of Var(base)
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_add_reg_array(tmp,loc_type,node->str,"",base);
		gen_restore(save);
		*offset = off;
//		printf("; gen_array_laddress 1.5: return %d,%s\n",*offset,tmp);
//		dump_loc_table();
		return tmp;
	}else if(isVAR(node->lhs)){		// 添字が単純変数
		char	*v1 = node->str;
		char	*v2 = node->lhs->str;
		char	*tmp1;
		// TMPnに計算結果があった
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			*offset = 0;
//			printf("; gen_array_laddress 2: return %d,%s\n",*offset,tmp1);
			return	tmp1;
		}
		gen_save(save);
		LDD_V(node->lhs->str);
		if(isARRAY2(node)){
			ASLD();
		}
		ADD_V(node->str);
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_add_reg_array(tmp,loc_type,v1,v2,0);			// offsetは0で良い
		gen_restore(save);
		*offset = 0;
//		printf("; gen_array_laddress 3: return %d,",*offset);print_nodes_ln(node);
		return	tmp;
	}else if((node->lhs->kind==ND_ADD)
		&&   isVAR(node->lhs->lhs) && isNUM(node->lhs->rhs)
		&&   (node->lhs->rhs->val>=0) && (node->lhs->rhs->val<offset1)){
		// V+var+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 V (+ (ND_VAR var) (ND_NUM offset)))
		char	*v1 = node->str;
		char	*v2 = node->lhs->lhs->str;
		Node	*expr = node->lhs->lhs;
		int		val = node->lhs->rhs->val;
//		printf("; array type V%sexpr+offset) optimize\n",isARRAY1(node)?":":"(");fflush(stdout);
		char	*tmp1;
		// TMPnに計算結果がある
		if((tmp1=lv_search_tmp_array(loc_type,v1,v2,0))!=NULL){
			*offset = val*scale;
//			printf("; gen_array_laddress 4: return %d,%s\n",*offset,tmp1);
			return	tmp1;
		}
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		if(isARRAY2(node)){
			ASLD();
		}
		ADD_V(v1);					// 配列の添字はAccABにある
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		gen_restore(save);
		lv_add_reg_array(tmp,loc_type,v1,v2,0);			// offsetは0で良い
		*offset = val*scale;
//		printf("; gen_array_laddress 5: return %d,%s\n",*offset,tmp);
		return	tmp;
	}else if((node->lhs->kind==ND_ADD
		&&    isNUM(node->lhs->rhs) && node->lhs->rhs->val>=0 && node->lhs->rhs->val<offset1)){
		// V+expr+offsetの場合、offset計算は外に出せる(ただしoffsetが小さい時)
		// (ND_ARRAY1or2 V (+ (expr) (ND_NUM offset)))
		char *v1 = node->str;
		Node *expr = node->lhs->lhs;
		int	val = node->lhs->rhs->val;
//		printf("; array type V%sexpr+offset) optimize\n",loc_char);fflush(stdout);
		gen_save(save);
		gen_expr(expr);				// offset以外の添字の計算
		if(isARRAY2(node)){
			ASLD();
		}
		ADD_V(v1);					// 配列の添字はAccABにある
		char *tmp = lv_search_free_tmp();
		STD_V(tmp);
		lv_add_reg_array(tmp,LOC_LADRS,"","",0);			// offsetは0で良い
		gen_restore(save);
		*offset = val*scale;
//		printf("; gen_array_laddress 6: return %d,%s\n",*offset,tmp);
		return	tmp;
	}
	// 添字が一般の式の場合
	gen_save(save);
	gen_expr(node->lhs);	// 添字の計算をして
	if(isARRAY2(node)){
		ASLD();
	}
	ADD_V(node->str);		// 配列の添字はAccABにある
	char *tmp = lv_search_free_tmp();
	STD_V(tmp);
	lv_add_reg_array(tmp,LOC_LADRS,"","",0);			// offsetは0で良い
	gen_restore(save);
	*offset = 0;
//	printf("; gen_array_laddress 7: return %d,%s\n",*offset,tmp);
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
	// TODO:IXを壊しても良いなら、gen_array_laddressよりも効率的にアドレス計算できる場合があるが、
	// TMPの使い回しとどっちが効率的か簡単には判断できないのであとで考える
//	printf("; gen_array_address:");print_nodes_ln(node);
	int	offset;
	char *tmp = gen_array_laddress(node,save,&offset);
	LDX_V(tmp);
	lv_free_tmp_ladrs(tmp);	// LADRS情報は使ったら消して良い
	return	offset;
}

void
BEQNE(int cond,char *label)
{
	Bxx((cond?"EQ":"NE"),label);
}
void
BNEEQ(int cond,char *label)
{
	BEQNE(!cond,label);
}
void
BCCCS(int cond,char *label)
{
	Bxx((cond?"CC":"CS"),label);
}
void
BCSCC(int cond,char *label)
{
	BCCCS(!cond,label);
}
void
BLSHI(int cond,char *label)
{
	Bxx((cond?"LS":"HI"),label);
}
void
BHILS(int cond,char *label)
{
	BLSHI(!cond,label);
}
void
BPLMI(int cond,char *label)
{
	Bxx((cond?"PL":"MI"),label);
}
void
BMIPL(int cond,char *label)
{
	BPLMI(!cond,label);
}

//
//	条件演算の左右を入れ替えたときの条件を返す
//
NodeKind
flip_lr(NodeKind kind)
{
	NodeKind	newKind;

	switch(kind){
	case ND_EQ:	return kind;
	case ND_NE: return kind;
	case ND_GE: return ND_LE;
	case ND_GT: return ND_LT;
	case ND_LE: return ND_GE;
	case ND_LT: return ND_GT;
	default: 
		break;
	}
	printf("; reverse_relop:%d",kind);
	error("; reverse_relop: unknown node kind");
	return	0;
}
//
//	lhs-rhsの結果、
//	cond=1:条件が成立するならlabelへ
//	cond=0:条件が非成立ならlabelへ
//
void
gen8_branch_lr(NodeKind kind,int cond,char *label)
{
	switch(kind){
	case ND_EQ:	BEQNE(cond,label); break;
	case ND_NE:	BNEEQ(cond,label); break;
	case ND_GE:	BCCCS(cond,label); break;	// Acc>=M
	case ND_GT: BHILS(cond,label); break;	// Acc>M
	case ND_LE: BLSHI(cond,label); break;	// Acc<=M
	case ND_LT: BCSCC(cond,label); break;	// Acc<M
	default:	error("; gen8_branch what's happen?");
	}
}
//
//	rhs-lhsの結果、
//	cond=1:条件が成立するならlabelへ
//	cond=0:条件が非成立ならlabelへ
//
void
gen8_branch_rl(NodeKind kind,int cond,char *label)
{
	gen8_branch_lr(flip_lr(kind),cond,label);
}
//
//	条件演算が8bit演算で収まるなら生成して1を返す
//	ダメなら0を返す
//	cond: if_true  1
//		  if_false 1
//
int
gen8_skip_if(Node *node,int cond,char *label)
{
	if(node==NULL){
		return 0;
	}
	NodeKind	kind = node->kind;
	Node		*lhs = node->lhs;
	Node		*rhs = node->rhs;

	if(!isCompare(node)){
		return 0;
	}
//	printf("; gen8_skip_if:");print_nodes_ln(node);
	if(isNUM(lhs) && (lhs->val<0 || lhs->val>255)){
		return 0;
	}
	if(isNUM(rhs) && (rhs->val<0 || rhs->val>255)){
		return 0;
	}
	if(isVAR(lhs) || isVAR(rhs)){
		return 0;
	}
	if(isARRAY2(lhs) || isARRAY2(rhs)){
		return 0;
	}
	if((lhs->kind==ND_INKEY || lhs->kind==ND_KEYBOARD)
	&& isNUM(rhs)){	// unsigned 比較になるので注意
		if(kind==ND_INKEY){
			LDAA_I('_');
			JSR("ASCIN");
		}else{
			JSR("KBIN_SUB");
		}
		CMPB_I(rhs->val);
		gen8_branch_lr(kind,cond,label);
		return 1;
	}
	if(isARRAY1(lhs) && isNUM(rhs)){
		printf("; gen8_skip_if ARRAY1 NUM kind '%s'(%u),cond=%d\n",
											compare_symbol_str(kind),kind,cond);
		int  offset;
		offset=gen_array_address(node->lhs,"");
		LDAB_X0(offset);
		if(rhs->val!=0){
			CMPB_I(rhs->val);
		}
		gen8_branch_lr(kind,cond,label);
		return 1;
	}
	if(isARRAY1(lhs) && isARRAY1(rhs)){
		printf("; gen8_skip_if ARRAY1 simple_ARRAY1 kind '%s'(%u),cond=%d\n",
													compare_symbol_str(kind),kind,cond);
		int  offset1,offset2;
		char *tmp1,*tmp2;
		if((tmp1=gen_array_laddress(node->lhs,"",&offset1))==NULL){
			printf("; gen8_skip_if: ");print_nodes_ln(node->lhs);
			error("; gen8_skip_if: gen_arrary_laddress returns NULL");
		}
		if((tmp2=gen_array_laddress(node->rhs,"",&offset2))==NULL){
			printf("; gen8_skip_if: ");print_nodes_ln(node->lhs);
			error("; gen8_skip_if: gen_arrary_laddress returns NULL");
		}
		LDX_V(tmp1);
		LDAB_X0(offset1);
		LDX_V(tmp2);
		CMPB_X0(offset2);
		gen8_branch_lr(kind,cond,label);
		return 1;
	}
	return	0;
}
void gen_compare8(Node *node);
void gen_skip_if_false(Node *node,char *if_false);
void gen_skip_if_true(Node *node,char *if_true);

//
//	compare AccAB to node.
//	if false then branch to if_false
//
void
gen_branch_if_false(NodeKind kind,Node *node,int offset,char *if_false)
{
	char	*if_true = new_label();
	switch(kind){ // if lhs(Acc)-rhs(M) then if_true otherwise jump to if_false
	case ND_EQ:	CMPB_IVX(node,offset);
				BNE(if_false);
				CMPA_IVX(node,offset);
				BNE(if_false);
				break;
	case ND_NE:	CMPB_IVX(node,offset);
				BNE(if_true);
				CMPA_IVX(node,offset);
				BEQ(if_false);
				break;
	case ND_GE:	SUB_IVX(node,offset);		// lhs-rhs, AB>=M?
				BLT(if_false);				// AB<M (N=1) then false
				break;						// N=0, then true
	case ND_GT:	SUB_IVX(node,offset);		// lhs-rhs,	AB>M?
				BGT(if_true);				// AB>M then true
				BNE(if_false);				// AB<M then false
				TSTB();
				BEQ(if_false);				// AB==M then false
				break;
	case ND_LE:	SUB_IVX(node,offset);		// lhs-rhs, AB<=M?
				BLT(if_true);				// AB<M (N=1) then true
				BNE(if_false);				// AB>M then false
				TSTB();
				BNE(if_false);				// AB>M then false
				break;
	case ND_LT:	SUB_IVX(node,offset);		// lhs-rhs, AB<M?
				BGE(if_false);				// AB>=M (N=0) then false
				break;							// otherwise AB<M
	default:
				error("Bxx not known\n");
	}
	LABEL(if_true);
}
//
//	条件から短い分岐を作成する
//		条件成立しなかった時にif_falseに飛ぶ
//
void
gen_skip_if_false(Node *node,char *if_false)
{
//	printf("; gen_skip_if_false start\n");
	// > (ND_VAR I) 10 )
	if(node==NULL){
//		printf("; gen_skip_if_false node==NULL !\n");
		return;
	}
	if(gen8_skip_if(node,0,if_false)){
		return;
	}
	if(isVAR(node)){						// if 変数 の場合
		LDX_V(node->str);
		BEQ(if_false);
		return;
	}
	if(isNUM(node)){						// if定数
		if(node->val==0){
			BRA(if_false);
		}
		return;
	}
	if(isARRAY1(node)){
		int offset = gen_array_address(node,"");
		TSTany_X0(offset);
		BEQ(if_false);
		return;
	}
	if(isARRAY2(node)){
		int offset = gen_array_address(node,"");
		LDX_X(offset);
		BEQ(if_false);
		return;
	}
	NodeKind kind = node->kind;
	Node	*lhs = node->lhs;
	Node	*rhs = node->rhs;
	if(kind==ND_RELADD){		// 関係演算子同士のADD
#if	0
		if(!has_side_effect(rhs)){	// 右項に副作用がなければショートカット
			char *if_true = new_label();
			gen_skip_if_true(lhs,if_true);
			gen_skip_if_false(rhs,if_false);
			LABEL(if_true);
			return;
		}else if(!has_side_effect(lhs)){	// 左項に副作用がなければショートカット
			char *if_true = new_label();
			gen_skip_if_true(rhs,if_true);	// 右から処理する
			gen_skip_if_false(lhs,if_false);
			LABEL(if_true);
			return;
		}
#endif
		gen_compare8(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STAB_V1(tmp);
		gen_compare8(rhs);
		ADDB_V1(tmp);
		BEQ(if_false);
		return;
	}
	if(kind==ND_RELMUL){		// 関係演算子同士のMUL
#if	0
		if(!has_side_effect(rhs)){	// 右項に副作用がなければショートカット
			gen_skip_if_false(lhs,if_false);
			gen_skip_if_false(rhs,if_false);
			return;
		}else if(!has_side_effect(lhs)){	// 左項に副作用がなければショートカット
			gen_skip_if_false(rhs,if_false);	// 右から処理する
			gen_skip_if_false(lhs,if_false);
			return;
		}
#endif
		gen_compare8(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STAB_V1(tmp);
		gen_compare8(rhs);
		ANDB_V1(tmp);
		BEQ(if_false);
		lv_free_reg(tmp);
		return;
	}
	if(!isCompare(node)){					// 比較演算以外の一般の式
		gen_expr(node);
		int v;
		if(lv_search_reg_const("A",&v)){
			if(v){							// Aが非0(あるの?)
				return;						// 常に正なのでIFが成立してしまう
			}
			// A==0である
			TSTB();
			BEQ(if_false);
			return;
		}
		char *label = new_label();
		ABA();
		BNE(label);
		BCS(label);
		BRA(if_false);
		LABEL(label);
		return;
	}
//	printf("; gen_skip_if_false %s :",label);print_nodes_ln(node);
	char	*if_true = new_label();
	// 0との比較はサボれる場合がある
	if(isNUMx(rhs,0)
	&& (kind==ND_GE || kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(lhs)){
			if(lv_search_reg_var("D",lhs->str)){
				TSTA();					// 2 1
#if	EMU_CPU_BUGS
			}else if(lv_search_reg_var("X",lhs->str)){
				CPX_I(0);
#endif
			}else{
				TSTany_V0(lhs->str);
			}
		}else if(isARRAY(lhs) && is_simple_array(lhs)){// 単純配列の場合
			int	 offset;
			char *tmp;
			if((tmp=gen_array_laddress(lhs,"",&offset))==NULL){
				printf("; gen_skip_if_false: ");print_nodes_ln(lhs);
				error("; gen_skip_if_false: gen_arrary_laddress returns NULL");
			}
			LDX_V(tmp);
			TSTany_X0(offset);
		}else{
			gen_expr(lhs);
			TSTA();						// 直前がLDAAなら消せるが…
		}
		BPLMI(kind==ND_LT,if_false);
		return;
	}else if(isNUMorVAR(lhs) && isNUMorVAR(rhs) && isEQorNE(node)){	// ==,!= はCPXで
		LDX_IVX(lhs,0);
		if(!isNUMx(rhs,0)){	// CPX #0 は生成しない
			CPX_IV(rhs);
		}
		BNEEQ(kind==ND_EQ,if_false);
		return;
	}else if(isARRAY1(lhs) && isNUM(rhs) && isEQorNE(node)
			&& rhs->val>=0 && rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_skip_if_false ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X0(offset);
		if(rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(rhs->val);
		}
		BNEEQ(kind==ND_EQ,if_false);
		return;
	}else if(isARRAY2(lhs) && isNUMorVAR(rhs) && isEQorNE(node)){// 配列との ==,!= はCPXで
//		printf("; gen_skip_if_false ARRAY2 ==/!= NUMorVAR\n");
		int offset = gen_array_address(lhs,"");		// 配列アドレスがIXに入っている
		LDX_X(offset);
		if(!isNUMx(rhs,0)){	// CPX #0 は生成しない
			CPX_IV(rhs);
		}
		BNEEQ(kind==ND_EQ,if_false);
		return;
	}else if(isNUMorVAR(rhs)
		|| (isARRAY2(rhs) && is_simple_array(rhs))){// 右は定数か変数か単純配列
		gen_expr(lhs);			// AccABに結果がある
		int	 offset;
		if(isARRAY2(rhs)){
			char *tmp;
			if((tmp=gen_array_laddress(rhs,"D",&offset))==NULL){
				printf("; gen_skip_if_false: ");print_nodes_ln(rhs);
				error("; gen_skip_if_false: gen_arrary_laddress returns NULL");
			}
			LDX_V(tmp);
		}
//		printf("; gen_skip_if_false NUM or VAR or Simple Array:");print_nodes_ln(node);
		gen_branch_if_false(kind,rhs,offset,if_false);
		return;
	}else if (isARRAY2(lhs) && is_simple_array(lhs)){
		// 左が単純配列のときは左右入れ替えて生成
		Node *new = new_binary(flip_lr(kind),rhs,lhs);
		gen_skip_if_false(new,if_false);
		return;
    }else if (isARRAY1(lhs) && is_simple_array(lhs)
		  &&  isARRAY1(rhs) && is_simple_array(rhs)){	// 左も右も単純配列
		error("; gen_skip_if_false: It will be processed by gen8, so it should not go through here.");
		return;
	}else{
//		printf("; gen_skip_if_false:");print_nodes_ln(node);
		gen_expr(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STD_V(tmp);
		gen_expr(rhs);
//		printf("; gen_skip_if_false expr expr:");print_nodes_ln(node);
		Node *rnode = new_node_var(tmp);
		gen_branch_if_false(flip_lr(kind),rnode,0,if_false);
		lv_free_reg(tmp);
		return;
	}
	error("; gen_skip_if_compare error\n");
}

void
gen_branch_if_true(NodeKind kind,Node *node,int offset,char *if_true)
{
	char *if_false = new_label();
	switch(kind){ // if cc then jump to label otherwise jump to if_true
	case ND_EQ:	CMPB_IVX(node,offset);	// AB==M?
				BNE(if_false);
				CMPA_IVX(node,offset);
				BEQ(if_true);
				break;
	case ND_NE:	CMPB_IVX(node,offset);	// AB!=M?
				BNE(if_true);
				CMPA_IVX(node,offset);
				BNE(if_true);
				break;
	case ND_GE:	SUB_IVX(node,offset);	// lhs-rhs, AB>=M?
				BGE(if_true);			// AB>=M (N=0) then true
				break;
	case ND_GT:	SUB_IVX(node,offset);	// lhs-rhs, AB>M?
				BGT(if_true);			// AB<M then false
				BNE(if_false);			// AB>M	then true
				TSTB();
				BNE(if_true);			// AB>M then true
				break;
	case ND_LE:	SUB_IVX(node,offset);	// lhs-rhs, AB<=M?
				BLT(if_true);			// AB>M	then false
				BNE(if_false);			// AB<M then true
				TSTB();
				BEQ(if_true);			// AB==M then true
				break;
	case ND_LT:	SUB_IVX(node,offset);	// lhs-rhs, AB<=M?
				BLT(if_true);			// AB<M	 then true
				break;
	default:
				error("Bxx not known\n");
	}
	LABEL(if_false);
}
//
//	条件から短い分岐を作成する
//		条件成立した時はif_trueに飛ぶ
//
void
gen_skip_if_true(Node *node,char *if_true)
{
//	printf("; gen_skip_if_true start\n");
	// > (ND_VAR I) 10 )
	if(node==NULL){
		printf("; gen_skip_if_true node==NULL !\n");
		return;
	}
	//printf("; compare node? ");print_nodes_ln(node);
	if(gen8_skip_if(node,1,if_true)){
		return;
	}
	if(isVAR(node)){						// if 変数 の場合
		LDX_V(node->str);
		BNE(if_true);
		return;
	}
	if(isNUM(node)){						// if定数
		if(node->val!=0){
			BRA(if_true);
		}
		return;
	}
	if(isARRAY1(node)){
		int offset = gen_array_address(node,"");
		TSTany_X0(offset);
		BNE(if_true);
		return;
	}
	if(isARRAY2(node)){
		int offset = gen_array_address(node,"");
		LDX_X(offset);
		BNE(if_true);
		return;
	}
	NodeKind kind = node->kind;
	Node	*lhs = node->lhs;
	Node	*rhs = node->rhs;
	if(kind==ND_RELADD){				// 関係演算子同士のADD
#if	0
		if(!has_side_effect(rhs)){	// 右項に副作用がなければショートカット
			gen_skip_if_true(lhs,if_true);
			gen_skip_if_true(rhs,if_true);
			return;
		}else if(!has_side_effect(lhs)){	// 左項に副作用がなければショートカット
			gen_skip_if_true(rhs,if_true);	// 右から処理する
			gen_skip_if_true(lhs,if_true);
			return;
		}
#endif
		gen_compare8(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STAB_V1(tmp);
		gen_compare8(rhs);
		ADDB_V1(tmp);
		BNE(if_true);
		return;
	}
	if(kind==ND_RELMUL){		// 関係演算子同士のMUL
#if	0
		if(!has_side_effect(rhs)){	// 右項に副作用がなければショートカット
			char *if_false = new_label();
			gen_skip_if_false(lhs,if_false);
			gen_skip_if_true(rhs,if_true);
			LABEL(if_false);
			return;
		}else if(!has_side_effect(lhs)){	// 左項に副作用がなければショートカット
			char *if_false = new_label();
			gen_skip_if_false(rhs,if_false);	// 右から処理する
			gen_skip_if_true(lhs,if_true);
			LABEL(if_false);
			return;
		}
#endif
		gen_compare8(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STAB_V1(tmp);
		gen_compare8(rhs);
		ANDB_V1(tmp);
		BNE(if_true);
		lv_free_reg(tmp);
		return;
	}
	if(!isCompare(node)){					// 比較演算以外の一般の式
		gen_expr(node);
		int v;
		if(lv_search_reg_const("A",&v)){
			if(v){							// Aが非0(あるの?)
				BRA(if_true);
				return;						// 常に正なのでIFが成立してしまう
			}
			// A==0である
			TSTB();
			BNE(if_true);
			return;
		}
		ABA();
		BNE(if_true);
		BCS(if_true);
		return;
	}
//	printf("; gen_skip_if_true %s :",label);print_nodes_ln(node);
	char	*if_false = new_label();
	// 0との比較はサボれる場合がある
	if(isNUMx(rhs,0)
	&& (kind==ND_GE || kind==ND_LT)){ // >=0 と <0 の場合は bit15(N flag)を見れば良い
		if(isVAR(lhs)){
			if(lv_search_reg_var("D",lhs->str)){
				TSTA();
#ifdef	EMU_CPU_BUGS
			}else if(lv_search_reg_var("X",lhs->str)){
				CPX_I(0);
#endif
			}else{
				TSTany_V0(lhs->str);	// 6cycle,3byte
			}
		}else if(isARRAY(lhs) && is_simple_array(lhs)){// 単純配列の場合
			int	 offset;
			char *tmp;
			if((tmp=gen_array_laddress(lhs,"",&offset))==NULL){
				printf("; gen_skip_if_true: ");print_nodes_ln(lhs);
				error("; gen_skip_if_true: gen_arrary_laddress returns NULL");
			}
			LDX_V(tmp);
			TSTany_X0(offset);
		}else{
			gen_expr(lhs);
			TSTA();
		}
		BMIPL(kind==ND_LT,if_true);
		return;
	}else if(isNUMorVAR(lhs) && isNUMorVAR(rhs) && isEQorNE(node)){	// ==,!= はCPXで
		LDX_IVX(lhs,0);
		if(!isNUMx(rhs,0)){	// CPX #0 は生成しない
			CPX_IV(rhs);
		}
		BEQNE(kind==ND_EQ,if_true);
		return;
	}else if(isARRAY1(lhs) && isNUM(rhs) && isEQorNE(node)
			&& rhs->val>=0 && rhs->val<=255){	// 間接1バイトとの ==,!=
//		printf("; gen_skip_if_true ARRAY1 ==/!= 0-255\n");
		int offset = gen_array_address(lhs,"");		// 配列アドレスがIXに入っている
		LDAB_X0(offset);
		if(rhs->val){			// LDAはZフラグが立つので0との比較は省略
			CMPB_I(rhs->val);
		}
		BEQNE(kind==ND_EQ,if_true);
		return;
	}else if(isARRAY2(lhs) && isNUMorVAR(rhs) && isEQorNE(node)){// 配列との ==,!= はCPXで
//		printf("; gen_skip_if_true ARRAY2 ==/!= NUMorVAR\n");
		int offset = gen_array_address(lhs,"");		// 配列アドレスがIXに入っている
		LDX_X(offset);
		if(!isNUMx(rhs,0)){	// CPX #0 は生成しない
			CPX_IV(rhs);
		}
		BEQNE(kind==ND_EQ,if_true);
		return;
	}else if(isNUMorVAR(rhs)									// 左は式、右は定数か変数
	      ||(isARRAY2(rhs) && is_simple_array(rhs))){	// もしくは単純配列
		gen_expr(lhs);					// AccABに結果がある
		int	 offset;
		if(isARRAY2(rhs)){
			char *tmp;
			if((tmp=gen_array_laddress(rhs,"D",&offset))==NULL){
				printf("; gen_skip_if_true: ");print_nodes_ln(rhs);
				error("; gen_skip_if_true: gen_arrary_laddress returns NULL");
			}
			LDX_V(tmp);
		}
		gen_branch_if_true(kind,rhs,offset,if_true);
		return;
	}else if (isARRAY2(lhs) && is_simple_array(lhs)){
		// 左が単純配列のときは左右入れ替えて生成
		Node *new = new_binary(flip_lr(kind),rhs,lhs);
		gen_skip_if_true(new,if_true);
		return;
	}else{
		gen_expr(lhs);
		char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
		STD_V(tmp);
		gen_expr(rhs);
		Node *rnode = new_node_var(tmp);
		gen_branch_if_true(flip_lr(kind),rnode,0,if_true);
		return;
	}
}

//
//	関係演算処理(結果はAccAB)
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
//	関係演算処理(結果はAccB)
//
void
gen_compare8(Node *node)
{
	// > (ND_VAR I) 10 )
	if(node==NULL){
		printf(";compare node==NULL !\n");
		return;
	}
	//printf("; compare node? ");print_nodes_ln(node);
	char *if_false = new_label();
	gen_skip_if_false(node,if_false);
	LDAB_I(1);
	SKIP1();
	LABEL(if_false);
	CLRB();
	lv_free_reg_B();
	return;
}

//
//	乗算をシフト演算に変換する
//		変換できたら!=0、できなければ0
//		左項はgen_exprで展開済みで、AccABに入っている
//		左項が変数のときは、varに変数名が入ってくる（その他はvar==NULL)
//
int	MULtoSHIFT(Node *node,char *var)
{
//	printf("; MULtoSHIFT:%s,",(var==NULL?"NULL":var));print_nodes_ln(node);
	if(!isNUM(node)){
		return 0;
	}
	int	shift=0;
	char *tmp = var;
	switch(node->val){
	case 256:
		MUL256();
		return 1;
	case 11:
	case 13:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(1);		// x2=x*2
		ADD_V(tmp);		// x3=x2+1
		ASLD_N(2);		// x12=x3*4
		if(node->val==11){
			SUB_V(tmp);
		}else if(node->val==13){
			ADD_V(tmp);
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 20:
	case 10:
	case 5:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(2);
		ADD_V(tmp);
		if(node->val==20){
			ASLD();
			ASLD();
		}else if(node->val==10){
			ASLD();
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 12:
	case 6:
	case 3:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(1);
		ADD_V(tmp);
		if(node->val==12){
			ASLD();
			ASLD();
		}else if(node->val==6){
			ASLD();
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 18:
	case 9:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(3);
		ADD_V(tmp);
		if(node->val==18){
			ASLD();
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 14:
	case 7:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(3);
		SUB_V(tmp);
		if(node->val==14){
			ASLD();
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 17:
	case 15:
		if(var==NULL){
			tmp = lv_search_free_tmp();
			STD_V(tmp);
		}
		ASLD_N(4);
		if(node->val==15){
			SUB_V(tmp);
		}else{
			ADD_V(tmp);
		}
		if(var==NULL){
			lv_free_reg(tmp);
		}
		return 1;
	case 128:	shift++;
	case 64:	shift++;
	case 32:	shift++;
	case 16:	shift++;
	case 8:		shift++;
	case 4:		shift++;
	case 2:		shift++;
//		printf("; ASLD_N(%d)\n",shift);
		ASLD_N(shift);
		return 1;
	case 1:
		return 1;
	default:;
	}
	return 0;
}

void
gen_expr(Node *node)
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
//			printf("; gen_expr LDD_X(%d)\n",offset);
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
			LDAA_I('_');
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
	case ND_KEYBOARD:
			JSR("KBIN_SUB");
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
			Node	*lhs = node->lhs;
			if(lhs->kind==ND_DIV && isNUM(lhs->rhs) && lhs->rhs->val>0){	// 2の累乗で割る
//				printf("; DIV debug: ");print_nodes_ln(node);
				int	val=lhs->rhs->val;
				int	mask=val-1;
				char *label=new_label();
				switch(val){
				case	16384:
				case	8192:
				case	4096:
				case	2048:
				case	1024:
				case	512:
				case	256:
				case	128:
				case	64:
				case	32:
				case	16:
				case	8:
				case	4:
				case	2:
							gen_expr(lhs->lhs);
							if(val!=2){
								TSTA();
								BPL(label);
								NEGD();
								LABEL(label);
							}
							AND_I(mask);
							STD_V("MOD");
							return;
				case	1:	// 要る?
							CLRD();
							CLR_V("MOD");
							return;
				default: break;
				}
			}
			gen_expr(lhs);
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
	case ND_SUB: {
				gen_expr(node->lhs);
				if(isNUMorVAR(node->rhs)){
					SUB_IVX(node->rhs,0);
					return;
				}
				if(isARRAY2(node->rhs) && is_simple_array(node->rhs)){
					int offset;
					char *var = gen_array_laddress(node->rhs,"D",&offset);
					if (var!=NULL) {
						LDX_V(var);
						SUB_X(offset);
						return;
					}
				}
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
			if(isVAR(node->lhs)&&isVAR(node->rhs)		// 単純変数同士の乗算
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
					BNE(label);		// if 1, rhs is result.
					CLRD();					// otherwise 0
					LABEL(label);
					lv_free_reg_X();
					lv_free_reg(tmp);
				}
				return;
			}else if(isVAR(node->rhs)){
				gen_expr(node->lhs);
				int v;
				if(lv_search_reg_const("D",&v)){
					switch(v){
					case 0:	return;			// 左が0なら右は展開しなくて良い（けど副作用は？）
					case 1:	gen_expr(node->rhs);	// 1倍は右項そのまま
							return;			
					}
				}
				LDX_V(node->rhs->str);
				JSR("MULTIPLYX");
				return;
			}
			if(isNUM(node->rhs)){
				gen_expr(node->lhs);
				char *var=NULL;
				if(isVAR(node->lhs)){
					var = node->lhs->str;
				}
				if(MULtoSHIFT(node->rhs,var)){
					return;
				}
//				tmp = add_const_table(node->rhs->val);
				LDX_I(node->rhs->val);
				JSR("MULTIPLYX");
				return;
			}
			gen_expr(node->lhs);
			tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STD_V(tmp);
			gen_expr(node->rhs);
			LDX_V_I(tmp);
			JSR("MULTIPLY");
			lv_free_reg(tmp);
			}
			return;
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
					LDD_V(node->lhs->str);
					char *label  = new_label();
					BPL(label);
					ADD_I(1);
					LABEL(label);
					ASRD();
					return;
				}
				gen_expr(node->lhs);		// expr/2
				PSHB();						// 4 1
				ANDB_I(1);					// 2 2
				STAB_V1("MOD");				// 5 3
				CLR_V0("MOD");				// 6 3
				PULB();						// 4 1	↑21 10
				char *label  = new_label();
				TSTA();
				BPL(label);
				ADD_I(1);
				LABEL(label);
				ASRD();
				return;
			}
			// 定数での除算
			if(isNUM(node->rhs) && node->rhs->val==10){	// 10で割る時は特殊処理
				gen_expr(node->lhs);
				JSR("DIVS10");
				return;
			}
			if(isNUM(node->rhs) && node->rhs->val>0){	// 2の累乗で割る
//				printf("; DIV debug: ");print_nodes_ln(node);
				int	val=node->rhs->val;
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
							gen_expr(node->lhs);
							JSR("DIVPOW2");
							INS2();
							INS();
							return;
				case	1:	// 要る?
							CLR_V("MOD");
							gen_expr(node->lhs);
							return;
				default: break;
				}
			}
			gen_expr(node->lhs);	// 左項を計算
			if(isVAR(node->rhs)){
				LDX_V(node->rhs->str);
				JSR("RDIVIDEX");
				return;
			}
			if(isNUM(node->rhs)){
				LDX_I(node->rhs->val);
				JSR("RDIVIDEX");
				return;
			}
			if(isARRAY2(node->rhs) && is_simple_array(node->rhs)){
				int offset;
				char *var = gen_array_laddress(node->rhs,"D",&offset);
				if (var!=NULL) {
					LDX_V(var);
					JSR("RDIVIDEX");
					return;
				}
			}
			char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
			STD_V(tmp);
			gen_expr(node->rhs);
			LDX_V(tmp);
			JSR("DIVIDEX");
			lv_free_reg(tmp);
			}
			break;
	case ND_ABS: {
			char	*LABS = new_label();
			gen_expr(node->lhs);
			TSTA();
			BPL(LABS);
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
				Node *lhs = node->lhs;
				Node *rhs = node->rhs;
#if	0
				if(!has_side_effect(rhs)){	// 右項に副作用がなければショートカット
					char *if_false = new_label();
					gen_skip_if_false(lhs,if_false);
					gen_skip_if_false(rhs,if_false);
					LDAB_I(1);
					SKIP1();
					LABEL(if_false);
					CLRB();
					CLRA();
				}else if(!has_side_effect(lhs)){	// 左項に副作用がなければショートカット
					char *if_false = new_label();
					gen_skip_if_false(rhs,if_false);	// 右から処理する
					gen_skip_if_false(lhs,if_false);
					LDAB_I(1);
					SKIP1();
					LABEL(if_false);
					CLRB();
					CLRA();
				}else{
#else
				{
#endif
					char *if_false = new_label();
					gen_compare8(lhs);
					char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
					STAB_V1(tmp);
					gen_compare8(rhs);
					ANDB_V1(tmp);
					lv_free_reg(tmp);
					CLRA();
				}
				return;
			}
			break;
	case ND_RELADD: { //  関係演算同士の+ は普通の加算で良い
				printf("; gen_expr ND_RELADD:");print_nodes_ln(node);
				gen_compare8(node->lhs);
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STAB_V1(tmp);
				gen_compare8(node->rhs);
				ADDB_V1(tmp);
				CLRA();
				lv_free_reg(tmp);
				return;
			}
			break;
	default:
			printf("; ");print_nodes_ln(node);
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
		}else{
			lv_free_used();	// 一時利用領域クリア
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
			if(node->lhs->kind==ND_VAR){		// 変数への代入
				gen_expr(rhs);
				gen_store_var(lhs);
			}else if(isARRAY1(lhs)){		// var:x) への代入
				if(isNUM(rhs) && rhs->val==0){
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					CLR_X0(offset);
					return;
				}else if(isNUMorVAR(rhs)){
//					printf("; debug ND_ASSIGN ARRAY1=NUMorVAR");print_nodes_ln(node);
					int	offset = gen_array_address(lhs,"");	// X=adrs, offset=subscript
					LDAB_IVX(rhs,0);
					STAB_X0(offset);
					return;
				}else if(isARRAY1(rhs) && is_simple_array(rhs)){
					int	off1,off2;
					char *tmp2 = gen_array_laddress(rhs,"",&off2);
					char *tmp1 = gen_array_laddress(lhs,"",&off1);
					LDX_V(tmp2);
					LDAB_X0(off2);
					LDX_V(tmp1);
					STAB_X0(off1);
					return;
				}
#if	0
				if(!has_side_effect(lhs)){
//					printf("; ND_ASSIGN lhs no side effect");print_nodes_ln(lhs);
					gen_expr(rhs);
					int offset = gen_array_address(lhs,"B");
					STAB_X0(offset);
					return;
				}
#endif
#if	1
				int	offset;
				char *tmp;
				tmp = gen_array_laddress(lhs,"",&offset);
				gen_expr(rhs);
				LDX_V(tmp);
				STAB_X0(offset);
				return;
#else
				gen_expr(lhs->lhs);						// calculate subscript
				ADD_V(lhs->str);						// 左辺のアドレスはDにある
				char *tmp = lv_search_free_tmp();		// lhsはTMPに保存する
				STD_V(tmp);
				gen_expr(rhs);
				LDX_V(tmp);
				STAB_X0(0);
				lv_free_reg(tmp);
#endif
				return;
			}else if(isARRAY2(lhs)){
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
					int offset;
					char *tmp = gen_array_laddress(lhs,"",&offset);	// X=adrs, offset=subscript
					LDD_IVX(rhs);
					LDX_V(tmp);
					STD_X(offset);
					return;
				}else if(isARRAY2(rhs) && is_simple_array(rhs)){
					int	off1,off2;
					char *tmp2 = gen_array_laddress(rhs,"",&off2);
					char *tmp1 = gen_array_laddress(lhs,"",&off1);
					LDX_V(tmp2);
					LDD_X(off2);
					LDX_V(tmp1);
					STD_X(off1);
					return;
				}
				int  offset;
				char *tmp = gen_array_laddress(lhs,"",&offset);
				gen_expr(rhs);
				LDX_V(tmp);
				STD_X(offset);
				return;
			}else{
				error("ASSIGN VAR error not var/array\n");
			}
		}
		return;
	case ND_IF: {
			char *then		= new_label();			// THEN portion label
			char *next_line = new_line_label(current->rhs->val);
			gen_skip_if_true(node->lhs,then);
			JMP(next_line);
			LABEL(then);
		}
		return;
	case ND_IFGOTO: {
			char *goto_line	= new_line_label(node->val);
			char *if_false	= new_label();
			gen_skip_if_false(node->lhs,if_false);
			JMP(goto_line);
			LABEL(if_false);
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
	case ND_DO:{
			int v=node->val;
//			printf("; ND_DO %s opt=%d\n",odl[v].var,odl[v].opt);
			if(odl[v].opt){	// control var+arrayの初期化
//				printf("; ND_DO set control var %s\n",odl[v].var);
				for(int i=0; i<odl[v].n; i++){	// 使っているARRAYについて
//					printf("; ND_DO %s(%s)\n",odl[v].arrays[i],odl[v].var);
					int	scale = (odl[v].type[i]==ND_ARRAY1)?1:2;
					Node *new = new_unary(ND_SETVAR,
									new_binary(ND_ADD,
										new_node_var(odl[v].arrays[i]),
										new_binary(ND_MUL,	new_node_var(odl[v].var),
															new_node_num(scale))));
					new->str = odl[v].label[i];
					printf(";   ");print_nodes_ln(new);fflush(stdout);
					new = node_opt(new);
					printf("; =>");print_nodes_ln(new);fflush(stdout);
					gen_stmt(new);
				}
			}
			LABEL(new_do_label(v));
			lv_free_all();	// レジスタ割り当て全てクリア
		}
		return;
	case ND_UNTIL: {
			char	*DO_LOOP = new_do_label(node->val);
			char	*DO_SKIP = new_label();
			if(isCompare(node->lhs)){
//				printf("; until compre node: ");print_nodes_ln(node->lhs);
				char	*if_true = new_label();
				gen_skip_if_true(node->lhs,if_true);
				JMP(DO_LOOP);
				LABEL(if_true);
				return;
			}
//			printf("; until expr: ");print_nodes_ln(node->lhs);
			gen_expr(node->lhs);
			ABA();
			BNE(DO_SKIP);
			BVS(DO_SKIP);
			JMP(DO_LOOP);
			LABEL(DO_SKIP);
		}
		return;
	case ND_UPDATEDO: {
			int	v=node->val;
			if(odl[v].opt){	// control var+arrayの更新
//				printf("; ND_UNTIL control var %s increment\n",node->str);
				// 既にIXに値が入っている変数があれば、それを優先するように並べ替える
				for(int i=1; i<odl[v].n; i++){	// 使っているARRAYについて
					char *var = odl[v].label[i];
					if(lv_search_reg_var("X",var)==0)	continue;			// IXになければ無視
					// 先頭に持ってくる
					NodeKind	tmp_kind    = odl[v].type[i];
					char		*tmp_arrays = odl[v].arrays[i];
					char		*tmp_label  = odl[v].label[i];
					odl[v].type[i]   = odl[v].type[0];
					odl[v].arrays[i] = odl[v].arrays[0];
					odl[v].label[i]  = odl[v].label[0];
					odl[v].type[0]   = tmp_kind;
					odl[v].arrays[0] = tmp_arrays;
					odl[v].label[0]  = tmp_label;
					break;
				}
				for(int i=0; i<odl[v].n; i++){	// 使っているARRAYについて
					char *var = odl[v].label[i];
//					printf("; ND_UNTIL pseudo var %s increment\n",var);
					int scale = odl[v].step*((odl[v].type[i]==ND_ARRAY1)?1:2);
					Node *new = new_unary(ND_SETVAR,new_binary(ND_ADD,new_node_var(var),
																		new_node_num(scale)));
					new->str = var;
//					printf("; ND_UNTIL ");print_nodes_ln(new);
					new = node_opt(new);
//					printf(";      => ");print_nodes_ln(new);
					gen_stmt(new);
				}
			}
		}
		return;
	case ND_FOR: {
			// (ND_FOR J (ND_SETVAR J 0 ) 10 )
			// (ND_FOR str=I label=3 (ND_SETVAR str=I (ND_NUM 0)) ...)
			// 終値はoptimizerで+1されており、ND_NEXTでの比較はGEで行える
			char	*FOR_LABEL=new_for_label(node->val);
			char	*TO_LABEL=new_to_label(node->val);
			int		v = node->val;
			FORTO[v].type  = node->rhs->kind;
			FORTO[v].var   = node->str;
			FORTO[v].loop  = FOR_LABEL;
			FORTO[v].to    = TO_LABEL;
			gen_stmt(node->lhs);
//			printf("; ND_FOR %s opt=%d\n",node->str,ofl[v].opt);
			if(ofl[v].opt){	// control var+arrayの初期化
//				printf("; ND_FOR set control var %s\n",node->str);
				Node *init;
				if(node->lhs->kind==ND_SETVAR && isNUMorVAR(node->lhs->lhs)){
					init = node->lhs->lhs;
				}else if(node->lhs->kind==ND_ASSIGN && isNUMorVAR(node->lhs->rhs)){
					init = node->lhs->rhs;
				}else{
					init = new_node_var(node->str);
				}
				printf("; ND_FOR control var init\n");
//				printf(";    ");print_nodes_ln(init);
				init = const_opt(init);
//				printf(";  =>");print_nodes_ln(init);
				for(int i=0; i<ofl[v].n; i++){	// 使っているARRAYについて
					printf("; ND_FOR %s(%s)\n",ofl[v].arrays[i],node->str);
					int	scale = (ofl[v].type[i]==ND_ARRAY1)?1:2;
					Node *new = new_unary(ND_SETVAR,
									new_binary(ND_ADD,
										new_node_var(ofl[v].arrays[i]),
										new_binary(ND_MUL, init, new_node_num(scale))));
					new->str = ofl[v].label[i];
//					printf("; ND_FOR internal arrays\n");
//					printf(";   ");print_nodes_ln(new);fflush(stdout);
					new = node_opt(new);
//					printf("; =>");print_nodes_ln(new);fflush(stdout);
					gen_stmt(new);
				}
			}
//			printf("; ND_FOR end of conttol var %s\n",node->str);
//			dump_loc_table();
			// 終値は+1されている。数値のときはImmediateにするために、値を覚えておく
			if(isNUM(node->rhs)){
				FORTO[v].val = node->rhs->val;
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
		int		v = node->val;
		char	*FOR_LABEL=FORTO[v].loop;
		char	*TO_LABEL =FORTO[v].to;
		char	*NEXT_LABEL=new_label();
//		printf("; ND_NEXT ");print_nodes_ln(node);
//		printf("; optimize? %d\n",ofl[v].opt);
//		printf("; node->lhs->lhs:");print_nodes_ln(node->lhs->lhs);
		if(ofl[v].opt
		&& node->lhs->kind==ND_ADD
		&& strcmp(node->lhs->lhs->str,ofl[v].var)==0
		&& isNUM(node->lhs->rhs) 
		&& ((node->lhs->rhs->val==1)||(node->lhs->rhs->val==2))){	// control var+arrayの増分処理
//			printf("; ND_NEXT control var %s increment\n",node->str);
			// 既にIXに値が入っている変数があれば、それを優先するように並べ替える
			for(int i=1; i<ofl[v].n; i++){	// 使っているARRAYについて
				char *var = ofl[v].label[i];
				if(lv_search_reg_var("X",var)==0)	continue;			// IXになければ無視
				// 先頭に持ってくる
				NodeKind	tmp_kind    = ofl[v].type[i];
				char		*tmp_arrays = ofl[v].arrays[i];
				char		*tmp_label  = ofl[v].label[i];
				ofl[v].type[i]   = ofl[v].type[0];
				ofl[v].arrays[i] = ofl[v].arrays[0];
				ofl[v].label[i]  = ofl[v].label[0];
				ofl[v].type[0]   = tmp_kind;
				ofl[v].arrays[0] = tmp_arrays;
				ofl[v].label[0]  = tmp_label;
				break;
			}
			for(int i=0; i<ofl[v].n; i++){	// 使っているARRAYについて
				char *var = ofl[v].label[i];
//				printf("; ND_NEXT pseudo var %s increment\n",var);
				int scale = (ofl[v].type[i]==ND_ARRAY1)?1:2;
				Node *new = new_unary(ND_SETVAR,new_binary(ND_ADD,new_node_var(var),
												new_binary(ND_MUL,	node->lhs->rhs,
																	new_node_num(scale))));
				new->str = var;
//				printf("; ND_NEXT ");print_nodes_ln(new);
				new = node_opt(new);
//				printf(";      => ");print_nodes_ln(new);
				gen_stmt(new);
			}
		}
		gen_expr(node->lhs);		// LDX,INC,STX 5+4+6=15よりもLDD/ADD/STD 8+4+10=22が良い
		STD_V(node->str);			// 次のLDDが不要になるので
//		LDD_V(node->str);
		if(FORTO[v].type==ND_NUM){
			printf("; FORTO[node->val].val=%d\n",FORTO[v].val);
			SUB_I(FORTO[v].val);
		}else{
			SUB_V(TO_LABEL);
		}
		BGE(NEXT_LABEL);
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
				printf("; ND_SETVAR ");print_nodes_ln(node);
				int offset = gen_array_address(node->lhs,"");	// IXにA(I) or A:I)のアドレスが入る
				if(isARRAY1(node->lhs)){
					LDAB_X0(offset);
					CLRA();
					STD_V(node->str);
				}else if(isARRAY2(node->lhs)){
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
			}else if((node->lhs->kind==ND_ADD)
				  && (isVAR(node->lhs->lhs))
				  && (isNUM(node->lhs->rhs))
				  && (-2 <= node->lhs->rhs->val)
				  && (node->lhs->rhs->val <=2)){
					// (ND_SETVAR str=F_ML2_1 (+ (ND_VAR M) (ND_NUM 2)))
					// LDX/INX/STXだと5+4*n+6 = 15〜19
					// LDD/ADD/STDだと8+4+10  = 22
					char *v = node->lhs->lhs->str;
					if(lv_search_reg_var("X",v)){					// IXにあるか?
						INXDEX(node->lhs->rhs->val);
						STX_V(node->str);
					}else if(lv_search_reg_var("D",v)){				// Dにあるか
						ADD_I(node->lhs->rhs->val);
						STD_V(node->str);
					}else{
						LDX_V(node->lhs->lhs->str);
						INXDEX(node->lhs->rhs->val);
						STX_V(node->str);
					}
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
	for(int i=0; i<const_count; i++){
			FDB_V(const_table_s[i],const_table[i]);
	}
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
	for(int i=1; i<odl_n+1; i++){
		if(ofl[i].opt==0){
			continue;
		}
		for(int j=0; j<odl[i].n; j++){
			printf("_%s\tRMB\t2\t; do loop pseudo array\n",odl[i].label[j]);
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
