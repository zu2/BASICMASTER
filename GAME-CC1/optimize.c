#include	"common.h"

int	isNUM(Node *node)
{
	return(node->kind==ND_NUM);
}
int	isVAR(Node *node)
{
	return(node->kind==ND_VAR);
}
int	isNUMorVAR(Node *node)
{
	return(isNUM(node)||isVAR(node));
}

int	isSameNUM(Node *x,Node *y)
{
	return (isNUM(x) && isNUM(y) && (x->val==y->val));
}

int	isSameVAR(Node *x,Node *y)
{
	return( isVAR(x) && isVAR(y) && strcmp(x->str,y->str)==0 );
}

int	isARRAY1(Node *node)
{
	return(node->kind==ND_ARRAY1);
}
int	isARRAY2(Node *node)
{
	return(node->kind==ND_ARRAY2);
}
int	isARRAY(Node *node)
{
	return(isARRAY1(node)||isARRAY2(node));
}
int	isSameARRAY(Node *x,Node *y)
{
	if(isARRAY1(x) && isARRAY1(y)
	&&	(strcmp(x->str,y->str)==0)
	&&	isSameNUM(x->lhs,y->lhs)){
		return 1;
	}
	if(isARRAY1(x) && isARRAY1(y)
	&&	(strcmp(x->str,y->str)==0)
	&&	isSameVAR(x->lhs,y->lhs)){
		return 1;
	}
	if(isARRAY2(x) && isARRAY2(y)
	&&	(strcmp(x->str,y->str)==0)
	&&	isSameNUM(x->lhs,y->lhs)){
		return 1;
	}
	if(isARRAY2(x) && isARRAY2(y)
	&&	(strcmp(x->str,y->str)==0)
	&&	isSameVAR(x->lhs,y->lhs)){
		return 1;
	}
	return	0;
}


int	isADDorSUB(Node *node)
{
	return((node->kind==ND_ADD) || (node->kind==ND_SUB));
}

int	isCompare(Node *node)
{
	return(	(node->kind==ND_EQ)
		||	(node->kind==ND_NE)
		||	(node->kind==ND_LT)
		||	(node->kind==ND_LE)
		||	(node->kind==ND_GT)
		||	(node->kind==ND_GE));
}
int	isEQorNE(Node *node)
{
	return(	(node->kind==ND_EQ)
		||	(node->kind==ND_NE));
}

Node	*new_ASLD(Node *node, int shift)
{
		node = new_unary(ND_ASLD,node);
		node->val = shift;
		return node;
}
Node	*new_ANDI(Node *node, int mask)
{
		node = new_unary(ND_ANDI,node);
		node->val = mask;
		return node;
}
Node	*new_ANDI_MOD(Node *node, int mask)
{
		node = new_unary(ND_ANDI_MOD,node);
		node->val = mask;
		return node;
}

//
// Does the formula have side effects?
//
int	has_side_effect(Node *node)
{
	Node	*old = node;
	switch(node->kind){
	case ND_NONE:
	case ND_CELL:
	case ND_VAR:
	case ND_NUM:
	case ND_LINENUM:
	case ND_PRINTCR:
		return 0;
	case ND_DIV:
	case ND_MOD:
	case ND_INKEY:	// $
	case ND_INPUT:	// ?
	case ND_TIMER:
	case ND_CURSOR:
	case ND_CURSORADRS:
	case ND_KEYBOARD:
		return 1;
	default:
		break;
	}
	return has_side_effect(node->lhs) || has_side_effect(node->rhs);
}

Node	*node_opt(Node	*old)
{
	static	int	cc=0;
	Node	*opt;
	Node	*node = old;

	switch(old->kind){
	case ND_NONE:
	case ND_VAR:
	case ND_NUM:
	case ND_LINENUM:
	case ND_PRINTCR:
		return old;
	default:
		break;
	}
	if((node->lhs!=NULL && node->lhs->kind != ND_NONE)
	|| (node->rhs!=NULL && node->rhs->kind != ND_NONE)){
//		左辺と右辺をそれぞれoptimizeしたものを新たなノードとする
//		printf("; optimize new copy node %d\n",++cc);
		node = new_copy_node(old);
//		printf("; ");print_nodes(old);printf("\n");
		if(node->lhs!=NULL){
			node->lhs = node_opt(old->lhs);
		}
		if(node->rhs!=NULL){
			node->rhs = node_opt(old->rhs);
		}
	}

	if(node->kind==ND_NEG){
		if(isNUM(node->lhs)){
			return new_node_num(-(node->lhs->val));
		}
	}else if(node->kind==ND_ADD){
		if(isNUM(node->lhs) && (node->lhs->val==0)){				// 0との加算
			return node_opt(node->rhs);
		}
		if(isNUM(node->rhs) && (node->rhs->val==0)){				// 0との加算
			return node_opt(node->lhs);
		}
		if(isNUM(node->lhs) && isNUM(node->rhs)){					// 定数同士
			return new_node_num(node->lhs->val + node->rhs->val);
		}
		if(isSameVAR(node->lhs,node->rhs)){							// A+A
			return new_ASLD(node->lhs,1);
		}
		if(isCompare(node->lhs) && isCompare(node->rhs)){	// 比較演算の結果の加算 {0,1}+{0,1}
//			printf("; optimize ND_MUL Multiplication of the results of comparison operations\n");
			return new_binary(ND_RELADD, node->lhs, node->rhs);
		}
		// 左が定数または変数で、右がそれ以外なら入れ替える
		if(isNUMorVAR(node->lhs) && !isNUMorVAR(node->rhs)){
			node = new_copy_node(old);
			node->lhs = old->rhs;
			node->rhs = old->lhs;
			return	node;
		}
		// 左が配列で右が定数でも配列でもなければ入れ替える
		if(isARRAY(node->lhs) && (!isNUMorVAR(node->rhs) && !isARRAY(node->rhs))){
			node = new_copy_node(old);
			node->lhs = old->rhs;
			node->rhs = old->lhs;
			return	node;
		}
		// 左が定数の加算で、右も定数なら計算しておく
		if((node->lhs->kind==ND_ADD && isNUM(node->lhs->rhs))
		&& isNUM(node->rhs)){
//			printf("; ");print_nodes_ln(node);
//			printf("; folding constant addition +%d+%d\n",node->lhs->rhs->val,node->rhs->val);
			int	val = node->lhs->rhs->val + node->rhs->val;
			if(val==0){
				return node->lhs->lhs;
			}
			node = new_copy_node(node->lhs);
			node->rhs->val = val;
//			printf("; => ");print_nodes_ln(node);
			return node;
		}
	}else if(node->kind==ND_SUB){
		if(isNUM(node->lhs) && isNUM(node->rhs)){
			return new_node_num(node->lhs->val - node->rhs->val);
		}
		if(isNUM(node->rhs)){		// 定数減算は負数の加算にする
			return new_binary(ND_ADD, node->lhs, new_node_num(-(node->rhs->val)));
		}
		if(isNUMorVAR(node->lhs) && !isNUMorVAR(node->rhs)){
			node = new_binary(ND_ADD, node_opt(new_unary(ND_NEG,old->rhs)), old->lhs);
			return	node;
		}
	}else if(node->kind==ND_MUL){
//		printf("; optimize ND_MUL\n");
		if(isCompare(node->lhs) && isCompare(node->rhs)){	// 比較演算の結果の乗算 {0,1}*{0,1}
//			printf("; optimize ND_MUL Multiplication of the results of comparison operations\n");
			return new_binary(ND_RELMUL, node->lhs, node->rhs);
		}
		if(isNUM(node->lhs) && isNUM(node->rhs)){
			return new_node_num(node->lhs->val * node->rhs->val);
		}
		// 左辺が定数または変数で、右辺がそれ以外なら入れ替える
		if(isNUMorVAR(node->lhs) && !isNUMorVAR(node->rhs)){
			node = new_copy_node(old);
			node->lhs = old->rhs;
			node->rhs = old->lhs;
		}
		// 右辺が定数でなければ最適化できない
		if(!isNUM(node->rhs)){
			return node;
		}
//		printf("; optimize ND_MUL rhs val=%d\n",node->rhs->val);
		// 以下は右辺が定数値 (左辺が定数・変数でないときに注意）
		switch(node->rhs->val){
		case -32:	return new_ASLD(node_opt(new_unary(ND_NEG,node->lhs)),5);
		case -16:	return new_ASLD(node_opt(new_unary(ND_NEG,node->lhs)),4);
		case -8:	return new_ASLD(node_opt(new_unary(ND_NEG,node->lhs)),3);
		case -4:	return new_ASLD(node_opt(new_unary(ND_NEG,node->lhs)),2);
		case -2:	return new_ASLD(node_opt(new_unary(ND_NEG,node->lhs)),1);
		case -1:	return new_unary(ND_NEG,node->lhs);
		case 0:		return new_node_num(0);
		case 1:		return node->lhs;
		case 2:		return new_ASLD(node->lhs,1);
		case 4:		return new_ASLD(node->lhs,2);
		case 8:		return new_ASLD(node->lhs,3);
		case 16:	return new_ASLD(node->lhs,4);
		case 32:	return new_ASLD(node->lhs,5);
		default:	break;
		}
		if(isNUMorVAR(node->lhs)){		// 左辺が定数か変数の場合
			switch(node->rhs->val){
			case 3:		return new_binary(ND_ADD,new_ASLD(node->lhs,1),node->lhs);
			case 5:		return new_binary(ND_ADD, new_ASLD(node->lhs,2),node->lhs);
			case 10:	return new_ASLD(new_binary(ND_ADD, new_ASLD(node->lhs,2),node->lhs),1);
			default:	break;
			}
		}
		// 左辺が式の場合は一時領域が必要なので、ここでは最適化できない。残念
		return	node;
	}else if(node->kind==ND_DIV){
#if	0
		// MODへの副作用が書けない…
		if(node->lhs->kind==ND_NUM && node->rhs->kind==ND_NUM
		&& node->rhs->val!=0
		&& node->lhs->val==node->rhs->val){
			return new_node_num(1);
		}
#endif
	}else if(node->kind==ND_ASSIGN){
//		printf("; ND_ASSIGN optimize :");print_nodes_ln(node);
		// (ND_ASSIGN (ND_VAR A) (ND VAR A)) -> omit
		if(isSameVAR(node->lhs,node->rhs)){
			char	*buf=calloc(1,256);
//			sprintf(buf,"* empty assignment optimized %s=%s",node->lhs->str,node->rhs->str);
			return new_node_REM(buf);
		}
		// assign constant or simple variable
		// (ND_ASSIGN (ND_VAR B) (ND_NUM 1))
		if(isVAR(node->lhs) && isNUMorVAR(node->rhs)){
			opt = new_unary(ND_SETVAR,node->rhs);
			opt->str = node->lhs->str;
			return	node_opt(opt);
		}
		// simple self modify
		// (ND_ASSIGN (ND_VAR A) (NG_NEG (ND_VAR A))) -> (ND_NEGVAR A)
		if(isVAR(node->lhs) 
		&& (node->rhs->kind==ND_NEG)
		&& isVAR(node->rhs->lhs)
		&& isSameVAR(node->lhs,node->rhs->lhs)){
			opt = new_node(ND_NEGVAR);
			opt->str = node->lhs->str;
			return	node_opt(opt);
		}
	}else if(node->kind==ND_SETVAR){
//		printf("; optimize ND_SETVAR ");print_nodes_ln(node);
//		printf("; node_opt ND_SETVAR\n");
//		printf("; kind=%d, is ND_ADD=%d ?  ",node->lhs->kind,ND_ADD);
//					printf(" isVAR=%d ?\n",isVAR(node->lhs->lhs));
		if(node->lhs->kind==ND_ADD
		&& isVAR(node->lhs->lhs)
		&& strcmp(node->str,node->lhs->lhs->str)==0
		&& isNUM(node->lhs->rhs)){
		// (ND_SETVAR J (+ (ND_VAR J) 1 ))
//			printf("; node_opt ND_SETVAR val=%d\n",node->lhs->rhs->val);
			if(node->lhs->rhs->val==1){
				opt = new_node(ND_INCVAR);
				opt->str = node->str;
				return opt;
			}else if(node->lhs->rhs->val==2){
				opt = new_node(ND_INC2VAR);
				opt->str = node->str;
				return opt;
			}else if(node->lhs->rhs->val==-1){
				opt = new_node(ND_DECVAR);
				opt->str = node->str;
				return opt;
			}else if(node->lhs->rhs->val==-2){
				opt = new_node(ND_DEC2VAR);
				opt->str = node->str;
				return opt;
			}
		}else if(node->lhs->kind==ND_SUB
		&& isVAR(node->lhs->lhs)
		&& strcmp(node->str,node->lhs->lhs->str)==0
		&& isNUM(node->lhs->rhs)){
		// (ND_SETVAR J (- (ND_VAR J) 1 ))
//			printf("; node_opt ND_SETVAR val=%D\n",node->lhs->rhs->val);
			if(node->lhs->rhs->val==1){
				opt = new_node(ND_DECVAR);
				opt->str = node->str;
				return opt;
			}else if(node->lhs->rhs->val==2){
				opt = new_node(ND_DEC2VAR);
				opt->str = node->str;
				return opt;
			}
		}else if(node->lhs->kind==ND_NEG
		&& isVAR(node->lhs->lhs)
		&& strcmp(node->str,node->lhs->lhs->str)==0){
		// (ND_SETVAR Z (ND_NEG (ND_VAR Z)))
//			printf("; node_opt ND_SETVAR val=%D\n",node->lhs->rhs->val);
			opt = new_node(ND_NEGVAR);
			opt->str = node->str;
			return opt;
		}
	}else if(node->kind==ND_GT && isNUM(node->rhs)){ // (> expr 97) -> (>= expr 98)
		//	GT => GE
		Node *opt = new_copy_node(node);
		Node *num = new_copy_node(node->rhs);
		num->val = num->val+1;
		opt->kind = ND_GE;
		opt->rhs = num;
//		printf(";     ");print_nodes_ln(node);
//		printf("; => ");print_nodes_ln(opt);
		return node_opt(opt);
	}else if(node->kind==ND_LE && isNUM(node->rhs)){ // (<= expr 97) -> (< expr 98)
		// LE => LTにする
		Node *opt = new_copy_node(node);
		Node *num = new_copy_node(node->rhs);
		num->val = num->val+1;
		opt->kind = ND_LT;
		opt->rhs = num;
//		printf(";     ");print_nodes_ln(node);
//		printf("; => ");print_nodes_ln(opt);
		return node_opt(opt);
	}else if(isCompare(node)		// 比較演算は右を定数・変数にする
		&&  ( (isNUM(node->lhs) && !isNUM(node->rhs))
			||(isVAR(node->lhs) && !isNUMorVAR(node->rhs)))){
		Node	*opt = new_copy_node(node);
		opt->lhs = node->rhs;
		opt->rhs = node->lhs;
		return node_opt(opt);
	}
	return	node;
}

int	ofl_n = 0;

//
//	varは制御変数、Nodeはそれを使っている配列変数
//
void
optimize_for_array_add(int n,char *var, Node *node)
{
	int ofl_max = ofl[n].n;
	for(int i=0; i<ofl_max; i++){			// 登録済み?
		if(strcmp(ofl[n].arrays[i],node->str)==0
		&& (ofl[n].type[i]==node->kind)){	// Yes
			return;
		}
	}
	char	*s = calloc(1,16);				// 追加する
	sprintf(s,"%s%s%d_%d",node->str,var,(node->kind==ND_ARRAY1)?1:2,n);
	ofl[n].type[ofl_max] = node->kind;
	ofl[n].arrays[ofl_max] = node->str;
	ofl[n].label[ofl_max] = s;
	ofl[n].n++;
//	printf("; for loop var=%s array search: %s(%s) found.\n",var,node->str,var);
//	printf(";");print_nodes_ln(node);
}
//
//	FORの制御変数を使っている単純配列があるか
//		制御変数Iの場合、A(I),A(I+off) のような配列を指す
//
void
optimize_for_array_search(int n,char *var,Node *node)
{
//	printf("; optimize_for_array_search:");print_nodes_ln(node);
	switch(node->kind){
	case ND_NONE:
	case ND_NOP:
	case ND_VAR:
	case ND_NUM:
	case ND_LINENUM:
	case ND_PRINTCR:
		return;
	default:
		break;
	}
	// search (ND_ARRAY1/2 str=X (ND_VAR str=I))
	optimize_for_array_search(n,var,node->lhs);
	optimize_for_array_search(n,var,node->rhs);
	if(isARRAY(node)){
		if(isVAR(node->lhs)
		&& (strcmp(node->lhs->str,var)==0)){			// array(var)
			optimize_for_array_add(n,var,node);
		}else if((node->lhs->kind==ND_ADD)
			&&   isVAR(node->lhs->lhs)
			&&   (strcmp(node->lhs->lhs->str,var)==0)
			&&   isNUM(node->lhs->rhs)){				// array(var+NUM)
				 optimize_for_array_add(n,var,node);
		}
	}
}

void
rewrite_for_array(int n,char *var,Node *node)
{
//	printf("; rewrite_for_array %d,%s:",n,var);print_nodes_ln(node);
	switch(node->kind){
	case ND_NONE:
	case ND_NOP:
	case ND_VAR:
	case ND_NUM:
	case ND_LINENUM:
	case ND_PRINTCR:
		return;
	default:
		break;
	}
	// search (ND_ARRAY1/2 str=X (ND_VAR str=I))
	rewrite_for_array(n,var,node->lhs);
	rewrite_for_array(n,var,node->rhs);
	if(isARRAY(node)){
		if(isVAR(node->lhs)
		&& (strcmp(node->lhs->str,var)==0)){			// array:var) or array(var)
			int ofl_max = ofl[n].n;
			for(int i=0; i<ofl_max; i++){
				if(strcmp(ofl[n].arrays[i],node->str)==0
				&& (ofl[n].type[i]==node->kind)){	// 登録されてるはず
//					printf("; for loop array rewrite:\n");
//					printf(";   ");print_nodes_ln(node);
					node->str = ofl[n].label[i];
					node->lhs = new_node_num(0);
//					printf("; =>");print_nodes_ln(node);
					return;
				}
			}
			// what's happen?
		}else if((node->lhs->kind==ND_ADD)
			&&   isVAR(node->lhs->lhs)
			&&   strcmp(node->lhs->lhs->str,var)==0
			&&	 isNUM(node->lhs->rhs)){				// array(var+offset)
			int ofl_max = ofl[n].n;
			for(int i=0; i<ofl_max; i++){
				if(strcmp(ofl[n].arrays[i],node->str)==0
				&& (ofl[n].type[i]==node->kind)){	// 登録されているはず
//					printf("; for loop array rewrite:\n");
//					printf(";   ");print_nodes_ln(node);
					node->str = ofl[n].label[i];
					node->lhs = node->lhs->rhs;
//					printf("; =>");print_nodes_ln(node);
					return;
				}
			}
			// what's happen?
		}
	}
}

void
optimize_for_loop()
{
	// (ND_FOR J (ND_SETVAR str=J 0 ) 10 )
	// => (ND_FOR J (ND_SETVAR str=J 0 ) (+ 10 1) )
	// => (ND_FOR J (ND_SETVAR str=J 0 ) 11 )
	for(int i=0; code[i]; i++){
		Node *node = code[i];
		if(node->kind==ND_FOR){
			Node *new = new_binary(ND_ADD,node->rhs,new_node_num(1));
//			printf("; FOR node rewrite\n");fflush(stdout);
//			printf(";   ");print_nodes_ln(node);fflush(stdout);
//			printf(";   ");print_nodes_ln(node->rhs);fflush(stdout);
//			printf("; =>");print_nodes_ln(new);fflush(stdout);
			node->rhs = node_opt(new);
//			printf("; =>");print_nodes_ln(node->rhs);fflush(stdout);
		}
	}
	ofl_n = 0;
	for(int i=0; i<(int)(sizeof(ofl)/sizeof(opt_for_loop_t)); i++){
		int opt=0;
	}
	for(int i=0; code[i]; i++){
		Node *node_f = code[i];
		// XXX 行番号とGOTOのチェックを足す
		if(node_f->kind==ND_FOR){
//			printf("; for_loop begin:");print_nodes_ln(node_f);
			int n=node_f->val;
			if(n>ofl_n){
				ofl_n = n;
			}
			ofl[n].var = node_f->str;
			ofl[n].opt = 1;
			ofl[n].n   = 0;
//			printf("; for_loop seach next\n");
			for(int j=i+1; code[j]; j++){
				Node *node_n = code[j];
				// (ND_NEXT J (+ (ND_VAR J) 1 ))
				if((node_n->kind==ND_NEXT)
				&& (node_n->val==n)){		// 対応するNEXTがあった
//					printf("; step node %d %s :",n,ofl[n].var);print_nodes(node_n);
					if((node_n->lhs->kind==ND_ADD)	// STEP 1,2だけが最適化の対象
					&& isVAR(node_n->lhs->lhs)
					&& strcmp(node_n->lhs->lhs->str,ofl[n].var)==0
					&& isNUM(node_n->lhs->rhs)
					&& ((node_n->lhs->rhs->val==1)||(node_n->lhs->rhs->val==2))){
//						printf("; is optimized\n");
						break;				
					}
//					printf("; is not optimized\n");
					ofl[n].opt = 0;
					break;
				}
//				printf("; scan loop: ");print_nodes_ln(node_n);
				// (ND_SETVAR str=I (ND_NUM 0))
				if(node_n->kind==ND_SETVAR
				&& strcmp(node_n->str,node_f->str)==0){	// 制御変数への代入がある
					ofl[n].opt = 0;
					printf("; break control var:");print_nodes_ln(node_n);
					break;
				}
				// (ND_ASSIGN (ND_VAR B) (ND_NUM 1))
				if((node_n->kind==ND_ASSIGN)			// 制御変数への代入がある
				&& isVAR(node_n->lhs)
				&& strcmp(node_n->lhs->str,node_f->str)){
					ofl[n].opt = 0;
					printf("; break control var:");print_nodes_ln(node_n);
					break;
				}
				if((  (node_n->kind==ND_INCVAR)			// 制御変数への代入がある
					||(node_n->kind==ND_INC2VAR)
					||(node_n->kind==ND_DECVAR)
					||(node_n->kind==ND_DEC2VAR)
					||(node_n->kind==ND_NEGVAR))
				&& strcmp(node_n->str,node_f->str)){
					ofl[n].opt = 0;
					printf("; break control var:");print_nodes_ln(node_n);
					break;
				}
				if(node_n->kind==ND_GOSUB){				// GOSUBがあると制御変数が不確定
					printf("; find gosub:");print_nodes_ln(node_n);
					break;
				}
				optimize_for_array_search(n,node_f->str,node_n);
			}
		}else if(node_f->kind==ND_NEXT){
//			printf("; for_loop end:");print_nodes_ln(node_f);
		}
	}
//	printf("; for loop found\n");
	for(int i=1; i<ofl_n+1; i++){
		if(ofl[i].opt==0){		// dont optimize this loop
//			printf("; for loop optimize ommited for %d %s\n",i,ofl[i].var);
			continue;
		}
		if(ofl[i].n==0){		// 書き換え対象の配列がないので、これもoptimizeしない
			ofl[i].opt=0;
//			printf("; for loop optimize ommited for %d %s no arrays\n",i,ofl[i].var);
			continue;
		}
//		printf("; for loop optimize %s\n",ofl[i].var);
//		printf("; loop:  %d\n", i);
//		printf("; var:   %s\n", ofl[i].var);
		for(int j=0; j<ofl[i].n; j++){
			char *s = (ofl[i].type[j]==ND_ARRAY1)?":":"(";
//			printf("; array: %s%s%s+offset)\n",ofl[i].arrays[j],s,ofl[i].var);
//			printf("; label: %s\n",ofl[i].label[j]);
		}
	}
//	printf("; rewrite AST tree\n");
	for(int i=0; code[i]; i++){
		Node *node_f = code[i];
		if(node_f->kind==ND_FOR){
			int n=node_f->val;
			if(ofl[n].opt==0){		// dont optimize this loop
				continue;
			}
			for(int j=i+1; code[j]; j++){
				Node *node_n = code[j];
				// (ND_NEXT J (+ (ND_VAR J) 1 ))
				if((node_n->kind==ND_NEXT)
				&& (node_n->val==n)){		// 対応するNEXTがあったので書き換え終わり
					break;
				}
				rewrite_for_array(n,node_f->str,node_n);
			}
		}
	}
}
