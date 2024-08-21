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
		if(node->lhs!=NULL && node->lhs->kind != ND_NONE && node->lhs->kind!=ND_VAR && node->lhs->kind!=ND_NUM){
			node->lhs = node_opt(old->lhs);
		}
		if(node->rhs!=NULL && node->rhs->kind != ND_NONE && node->rhs->kind!=ND_VAR && node->rhs->kind!=ND_NUM){
			node->rhs = node_opt(old->rhs);
		}
	}

	if(node->kind==ND_NEG){
		if(isNUM(node->lhs)){
			return new_node_num(-(node->lhs->val));
		}
	}else if(node->kind==ND_ADD){
		if(isNUM(node->lhs) && isNUM(node->rhs)){
			return new_node_num(node->lhs->val + node->rhs->val);
		}
		if(isSameVAR(node->lhs,node->rhs)){							// A+A
			return new_ASLD(node->lhs,1);
		}
		/* 左辺が定数または変数で、右辺がそれ以外なら入れ替える */
		if(isNUMorVAR(node->lhs) && !isNUMorVAR(node->rhs)){
			node = new_copy_node(old);
			node->lhs = old->rhs;
			node->rhs = old->lhs;
			return	node;
		}
		// 左辺が定数の加算で、右辺が定数なら計算しておく
		if((node->lhs->kind==ND_ADD && isNUM(node->lhs->rhs))
		&& isNUM(node->rhs)){
			printf("; ");print_nodes_ln(node);
			printf("; Folding constant addition +%d+%d\n",node->lhs->rhs->val,node->rhs->val);
			int	val = node->lhs->rhs->val + node->rhs->val;
			node = new_copy_node(node->lhs);
			node->rhs->val = val;
			printf("; => ");print_nodes_ln(node);
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
			return new_binary(ND_BITAND, node->lhs, node->rhs);
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
	}else if((node->kind==ND_MOD)
	&&		 (isNUM(node->lhs))){			// %(定数)、MODを読む場合に使われる
	}else if((node->kind==ND_MOD)			// MOD演算で定数の場合
	&&   (node->lhs->kind==ND_DIV)
	&&    isVAR(node->lhs->lhs)
	&&    isNUM(node->lhs->rhs)){
		int	val=node->lhs->rhs->val;
//		printf("; MOD can optimize MOD(v/%d)? ",val);print_nodes_ln(node);
		if((val==2)  ||(val==4)   ||(val==8)   ||(val==16)
		|| (val==32) ||(val==64)  ||(val==128) ||(val==256)
		|| (val==512)||(val==1024)||(val==2048)||(val==4096)
		|| (val==8192)||(val==16384)||(val==32768)){
			return new_ANDI_MOD(node->lhs->lhs,val-1);
		}else if(val==-32768){
			return new_ANDI_MOD(node->lhs->lhs,32767);
		}
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
	}else if(isEQorNE(node)){		// ==, !== の定数・変数との比較は入れ替えておくと後が楽
		if((isNUM(node->lhs) && !isNUM(node->rhs))
		|| (isVAR(node->lhs) && !isNUM(node->rhs) && !isVAR(node->rhs))){
			Node	*opt = new_copy_node(node);
			opt->lhs = node->rhs;
			opt->rhs = node->lhs;
			return opt;
			return	opt;
		}
	}
	return	node;
}

//
//	Multi-Statement Optimization
//
void mso_sub(int lineno,int i)
{
	// 定数代入の連続 (ND_SETVAR str=E (ND_NUM 0))
	if(code[i]->kind==ND_SETVAR && isNUM(code[i]->lhs)){
		int	val = code[i]->lhs->val;
		int j=0;
		for(j=i+1;code[j];j++){
			if(code[j]->kind==ND_SETVAR
			&& isNUM(code[j]->lhs)
			&& (code[j]->lhs->val==val)){
//				printf("; continuous same constant assignment found  on line %d\n",lineno);
				continue;
			}
			break;
		}
		// 抜けてきたときにj>i+1なら最適化対象
		if(j>i+1){
//			printf("; continuous same constant assignment %d from line %d\n",j-i,lineno);
			Node	*node = new_node(ND_SETVAR_N);
			Node	*rhs_node = node;
			for(int k=i; k<j && code[k]->kind==ND_SETVAR; k++){
				Node *var = new_unary(ND_CELL,new_node_var(code[k]->str));
				rhs_node->rhs = var;
				rhs_node = var;
				code[k]->kind = ND_NOP;
			}
			rhs_node->rhs = new_node_none();
			node->lhs = node->rhs;		// ToDo: あとで書き換える
			node->rhs = code[i]->lhs;
//			printf("; ");print_nodes_ln(node);
			code[i]=node;
		}
	}else if(code[i]->kind==ND_SETVAR // 同一変数代入の連続 (ND_SETVAR str=E (ND_NUM 0))
	&& isVAR(code[i]->lhs)){
		char *str = code[i]->lhs->str;
		int j=0;
		for(j=i+1;code[j];j++){
			if(code[j]->kind==ND_SETVAR
			&& isVAR(code[j]->lhs)
			&& (strcmp(code[j]->lhs->str,str)==0)){
//				printf("; continuous same variable assignment found  on line %d\n",lineno);
				continue;
			}
			break;
		}
		// 抜けてきたときにj>i+1なら最適化対象
		if(j>i+1){
//			printf("; continuous same constant assignment %d from line %d\n",j-i,lineno);
			Node	*node = new_node(ND_SETVAR_N);
			Node	*rhs_node = node;
			for(int k=i; k<j && code[k]->kind==ND_SETVAR; k++){
				Node *var = new_unary(ND_CELL,new_node_var(code[k]->str));
				rhs_node->rhs = var;
				rhs_node = var;
				code[k]->kind = ND_NOP;
			}
			rhs_node->rhs = new_node_none();
			node->lhs = node->rhs;		// ToDo: あとで書き換える
			node->rhs = code[i]->lhs;
//			printf("; ");print_nodes_ln(node);
			code[i]=node;
		}
	}else if(code[i]->kind==ND_ASSIGN // (ND_ASSIGN (ND_ARRAY1 str=B (ND_NUM 0)) (ND_NUM 138))
		&&   code[i]->lhs->kind==ND_ARRAY1
		&&   isNUM(code[i]->lhs->lhs) && isNUMorVAR(code[i]->rhs)
		&&   (code[i]->lhs->lhs->val>=0) && (code[i]->lhs->lhs->val<255)){
		char *var = code[i]->lhs->str;
		int j=0;
		for(j=i+1;code[j];j++){
			if(code[j]->kind==ND_ASSIGN
			&& code[j]->lhs->kind==ND_ARRAY1
			&& strcmp(var,code[j]->lhs->str)==0
			&& isNUM(code[j]->lhs->lhs) && isNUM(code[j]->rhs)
			&& (code[j]->lhs->lhs->val>=0) && (code[j]->lhs->lhs->val<255)){
//				printf("; continuous same ARRAY1 assignment found %d on line %d, %s:%d)=%d\n",j,lineno,var,code[j]->lhs->lhs->val,code[j]->rhs->val);
				continue;
			}
			break;
		}
		// 抜けてきたときにj>i+1なら最適化対象
		if(j>i+1){
			// (ND_ASSIGN (ND_ARRAY1 str=B (ND_NUM 0)) (ND_NUM 138))
//			printf("; continuous same ARRAY1 assignment found %d-%d on line %d, %s:%d)=%d\n",i,j-1,lineno,var,code[i]->lhs->lhs->val,code[i]->rhs->val);
			Node	*node = new_node(ND_SETARY1_N);
			node->lhs = code[i]->lhs;		// lhsはそのまま下げる。lhs->lhsは不要だがそのまま
			char	*str = code[i]->lhs->str;
			Node	*rhs_node = node;
			for(int k=i; k<j && code[k]->kind==ND_ASSIGN; k++){
//				printf("; new pair ");print_nodes(code[k]->lhs->lhs);print_nodes_ln(code[k]->rhs);
				Node *pair = new_binary(ND_CELL,code[k]->lhs->lhs,code[k]->rhs); // offset,valのペア
//				printf("; pair ");print_nodes_ln(pair);
				Node *cell = new_unary(ND_CELL,pair);
//				printf("; cell ");print_nodes_ln(cell);
				rhs_node->rhs = cell;
				rhs_node = cell;
				code[k]->kind = ND_NOP;
			}
//			printf("; ");print_nodes_ln(node);
			code[i]=node;
		}
	}else if(code[i]->kind==ND_ASSIGN // (ND_ASSIGN (ND_ARRAY1 str=B (ND_NUM 0)) (ND_NUM 138))
		&&   code[i]->lhs->kind==ND_ARRAY2
		&&   isNUM(code[i]->lhs->lhs) && isNUMorVAR(code[i]->rhs)
		&&   (code[i]->lhs->lhs->val>=0) && (code[i]->lhs->lhs->val<127)){
		char *var = code[i]->lhs->str;
		int j=0;
		for(j=i+1;code[j];j++){
			if(code[j]->kind==ND_ASSIGN
			&& code[j]->lhs->kind==ND_ARRAY2
			&& strcmp(var,code[j]->lhs->str)==0
			&& isNUM(code[j]->lhs->lhs) && isNUM(code[j]->rhs)
			&& (code[j]->lhs->lhs->val>=0) && (code[j]->lhs->lhs->val<127)){
//				printf("; continuous same ARRAY1 assignment found %d on line %d, %s:%d)=%d\n",j,lineno,var,code[j]->lhs->lhs->val,code[j]->rhs->val);
				continue;
			}
			break;
		}
		// 抜けてきたときにj>i+1なら最適化対象
		if(j>i+1){
			// (ND_ASSIGN (ND_ARRAY1 str=B (ND_NUM 0)) (ND_NUM 138))
			printf("; continuous same ARRAY1 assignment found %d-%d on line %d, %s:%d)=%d\n",i,j-1,lineno,var,code[i]->lhs->lhs->val,code[i]->rhs->val);
			Node	*node = new_node(ND_SETARY2_N);
			node->lhs = code[i]->lhs;		// lhsはそのまま下げる。lhs->lhsは不要だがそのまま
			char	*str = code[i]->lhs->str;
			Node	*rhs_node = node;
			for(int k=i; k<j && code[k]->kind==ND_ASSIGN; k++){
				printf("; new pair ");print_nodes(code[k]->lhs->lhs);print_nodes_ln(code[k]->rhs);
				Node *pair = new_binary(ND_CELL,code[k]->lhs->lhs,code[k]->rhs); // offset,valのペア
				printf("; pair ");print_nodes_ln(pair);
				Node *cell = new_unary(ND_CELL,pair);
				printf("; cell ");print_nodes_ln(cell);
				rhs_node->rhs = cell;
				rhs_node = cell;
				code[k]->kind = ND_NOP;
			}
			printf("; ");print_nodes_ln(node);
			code[i]=node;
		}
	}
}

void
multi_statement_optimize()
{
	int lineno;
	for(int i=0; code[i]; i++){
		if(code[i]->kind==ND_LINENUM){
			lineno = code[i]->val;
			continue;
		}
		mso_sub(lineno,i);
	}
}
