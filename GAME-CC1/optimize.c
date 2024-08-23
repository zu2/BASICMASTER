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
	case ND_VAR:
	case ND_NUM:
	case ND_LINENUM:
	case ND_PRINTCR:
		return 0;
	case ND_DIV:
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
