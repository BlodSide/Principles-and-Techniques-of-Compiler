#include "semantic.h"

//High-level Definitions
void Program( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	//Program: ExtDefList
	ExtDefList( p->child );
	checkFuncDef();
}

void ExtDefList( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	if( p->child == NULL ){
		return;
	}
	//ExtDefList: ExtDef ExtDefList
	ExtDef( p->child );
	ExtDefList( p->child->bro );
}

void ExtDef( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	SymDes* type;
	type = Specifier( child );
	child = child->bro;
	if( strcmp(child->name, "ExtDecList")==0 ){
		//ExtDef: Specifier ExtDecList SEMI
		ExtDecList( child, type );
	}
	else if( strcmp(child->name, "SEMI")==0 ){
		//ExtDef: Specifier SEMI
		return;
	}
	else{
		//ExtDef: Specifier FunDec xxx
		tablesPush();
		FuncDes* f = FunDec( child, type );
		assert( f!=NULL );
		/*
		 * 为了允许函数声明的存在 产生式在附录A的基础上增加
		 * ExtDef: Specifier FunDec SEMI
		 */
		child = child->bro;
		int flag = 0;
		if( strcmp(child->name, "SEMI")==0 ){
			//ExtDef: Specifier FunDec SEMI
			f->isDef = FALSE;
		}
		else{
			//ExtDef: Specifier FunDec CompSt
			f->isDef = TRUE;
			CompSt( child, type );
		}
		tablesPop();
		flag = insertFuncTable(f);
		assert( f!=NULL );
		switch( flag ){
			case REDEFINE_ERROR:
				printf( "Error type 4 at line %d: Redefined function \"%s\".\n", f->lineno, f->name );
				break;
			case INCONSISTENT_DELARATION_ERROR:
				printf( "Error type 19 at line %d: Inconsistent declaration of function \"%s\".\n", f->lineno, f->name );
				break;
		}
		
	}
}

void ExtDecList( Node* p, SymDes* type ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//ExtDecList: VarDec (COMMA ExtDecList)
	if( child->bro !=NULL ){
		SymDes* typeCpy = malloc( sizeof(SymDes) );
		memcpy( typeCpy, type, sizeof(SymDes) );
		ExtDecList( child->bro->bro, typeCpy );
	}
	VarDec( child, type, VAR_DEC_IN_EXT);
}

//Specifiers
SymDes* Specifier( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	SymDes* type;
	if( strcmp(child->name, "TYPE")==0 ){
		//Specifier: TYPE
		type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->kind = BASIC_KIND;
		if( strcmp(child->value,"int")==0 )
			type->detail.basic = INT_TYPE;
		else if( strcmp(child->value,"float")==0 )
			type->detail.basic = FLOAT_TYPE;
	}
	else{
		//Specifier: StructSpecifier
		type = StructSpecifier(child);
	}
	return type;
}

SymDes* StructSpecifier( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	StrucDes* struc;
	SymDes* type;
	Node* child = p->child->bro;
	if( strcmp(child->name, "OptTag")==0 ){
		//StructSpecifier: STRUCT OptTag LC DefList RC
		struc = malloc( sizeof(StrucDes) );
		memset( struc, 0, sizeof(StrucDes) );
		type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->detail.struc = struc;
		type->kind = STRUCT_KIND;
		
		if(child->child!=NULL)		
			struc->name = child->child->value;
		if( insertStrucTable( struc )== REDEFINE_ERROR){
			printf( "Error type 16 at line %d: Duplicated name \"%s\".\n", child->lineno, struc->name );
			return type;
		}
		child = child->bro->bro;
		tablesPush();
		struc->fieldList = DefList(child, VAR_DEC_IN_STRUCT);
		tablesPop();
	}
	else{
		//StructSpecifier: STRUCT Tag
		struc = findStrucDes( child->child->value );
		type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->detail.struc = struc;
		type->kind = STRUCT_KIND;
		if( struc==NULL ){
			printf( "Error type 17 at line %d: Undefined structure \"%s\".\n", child->lineno, child->child->value );
			return type;
		}
	}
	return type;
}

//Declarators
SymDes* VarDec( Node* p, SymDes* type, int context ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	if( strcmp(child->name,"ID")==0 ){
		//VarDec: ID
		type->name = child->value;
		if( insertSymTable( type )==REDEFINE_ERROR ){
			if( context==VAR_DEC_IN_EXT || context==VAR_DEC_IN_LOCAL || context==VAR_DEC_IN_PARAM )
				printf( "Error type 3 at line %d: Redefined variable \"%s\".\n", child->lineno, type->name );
			else
				printf( "Error type 15 at line %d: Redefined field \"%s\".\n", child->lineno,type->name );
			return NULL;
		}
		return type;
	}
	else{
		//VarDec: VarDec LB INT RB
		SymDes* upperType = malloc( sizeof(SymDes) );
		memcpy( upperType, type, sizeof(SymDes) );
		upperType->detail.array.formBy = type;
		upperType->kind = ARRAY_KIND;
		upperType->detail.array.size = atoi(child->bro->bro->value);
		return VarDec(child, upperType, context);
	}
}

FuncDes* FunDec( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//FunDec: ID LP (VarList) RP
	FuncDes* func = malloc( sizeof(FuncDes) );
	memset( func, 0, sizeof(FuncDes) );
	func->name = child->value;
	func->lineno = child->lineno;
	func->retType = retType;
	child = child->bro->bro;
	if( child->bro!=NULL ){
		func->paraList = VarList(child);
	}
	return func;
}

SymDes*  VarList( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//VarList: ParamDec (COMMA VarList)
	SymDes* left = ParamDec( child );
	child = child->bro;
	if( child!=NULL ){
		child = child->bro;
		//left可能由于一些错误导致为空
		if( left==NULL ){
			left = VarList( child );
		}
		else{
			SymDes* tail = left;
			while( tail->nextInList!=NULL )
				tail = tail->nextInList;
			tail->nextInList = VarList(child);
		}
	}
	return left;
}

SymDes* ParamDec( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//ParamDec: Specifier VarDec
	SymDes* type = Specifier(child);
	SymDes* para = VarDec( child->bro, type, VAR_DEC_IN_PARAM );
	return para;
}

/*Statements*/
void CompSt( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child->bro;
	//CompSt: LC DefList StmtList RC
	DefList( child, VAR_DEC_IN_LOCAL );
	child = child->bro;
	StmtList( child, retType );
}

void StmtList( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	if( child==NULL ){
		return;
	}
	//StmtList: Stmt StmtList
	Stmt( child, retType );
	StmtList( child->bro, retType );
}

void Stmt( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	if( strcmp(child->name,"Exp")==0 ){
		//Stmt: Exp SEMI
		Exp( child );
	}
	else if( strcmp(child->name,"CompSt")==0 ){
		//Stmt: CompSt
		tablesPush();
		CompSt( child, retType );
		tablesPop();
	}
	else if( strcmp(child->name,"RETURN")==0 ){
		//Stmt: RETURN Exp SEMI
		child = child->bro;
		SymDes* ret = Exp( child );
		if( retType==NULL && ret==NULL )
			return;
		else if( retType==NULL || ret==NULL || typeCmp( retType, ret )==FALSE ){
			printf( "Error type 8 at line %d: Type mismatched for return.\n", child->lineno );
		}
		return;
	}
	else if( strcmp(child->name,"WHILE")==0 ){
		//Stmt: WHILE LP Exp RP Stmt
		child = child->bro->bro;
		SymDes* whileExp = Exp( child );
		Stmt( child->bro->bro, retType );
	}
	else{
		//Stmt: IF LP Exp RP Stmt (ELSE Stmt)
		child = child->bro->bro;
		SymDes* whileExp = Exp( child );
		child = child->bro->bro;
		Stmt( child, retType );
		if( child->bro!=NULL ){
			Stmt( child->bro->bro, retType );
		}
	}
}

/*Local Definitions*/
SymDes* DefList( Node* p, int context ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	if( child==NULL )
		return NULL;
	//DefList: Def DefList
	SymDes* left = Def( child, context );
	SymDes* right = DefList( child->bro, context );
	SymDes* tail = left;
	if( tail!=NULL ){
		while( tail->nextInList!=NULL )
			tail = tail->nextInList;
		tail->nextInList = right;
	}
	else
		left = right;
	return left;
}

SymDes* Def( Node* p, int context ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//Def: Specifier DecList SEMI
	SymDes* type = Specifier( child );
	return DecList( child->bro, type, context );
}

SymDes* DecList( Node* p, SymDes* type, int context ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//DecList: Dec (COMMA DecList)
	SymDes* left = NULL;
	SymDes* right = NULL;
	if( child->bro!=NULL ){
		SymDes* typeCpy = malloc( sizeof(SymDes) );
		memcpy( typeCpy, type, sizeof(SymDes) );
		right = DecList( child->bro->bro, typeCpy, context );
	}
	left = Dec( child, type, context );
	if( left!=NULL )
		left->nextInList = right;
	else
		left = right;
	return left;
}

SymDes* Dec( Node* p, SymDes* type, int context ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	//Dec: VarDec (ASSIGNOP Exp)
	SymDes* left = VarDec( child, type, context );
	child = child->bro;
	if( child!=NULL ){
		if( context==VAR_DEC_IN_STRUCT ){
			printf( "Error type 15 at line %d: be initialized field \"%s\".\n", child->lineno, left->name );
			return left;
		}
		child = child->bro;
		SymDes* right = Exp(child);
		if( left!=NULL && right!=NULL && typeCmp( left, right )==FALSE ){
			printf( "Error type 5 at line %d: Type mismatched for assignment.\n", child->lineno );
		}
	}
	return left;
}

/*Expressions*/
SymDes* Exp( Node* p ){
	assert( p!=NULL );
	//printf("%s\n",p->name);
	Node* child = p->child;
	SymDes* retType=NULL;
	if( strcmp(child->name,"Exp")==0 ){
		if( strcmp(child->bro->name,"ASSIGNOP")==0 ){
			//Exp: Exp ASSIGNOP Exp
			Node* leftGrandChild = child->child;
			// 以下判断左表达式是否合法
			// 左表达式只可能是普通变量、数组访问表达式、结构体访问表达式
			SymDes* leftType = NULL;
			if( strcmp(leftGrandChild->name,"ID")==0 && leftGrandChild->bro==NULL ){
				//普通变量
				leftType = Exp( child );
			}
			else if( strcmp(leftGrandChild->name,"Exp")==0 && leftGrandChild->bro!=NULL && strcmp(leftGrandChild->bro->name,"LB")==0 ){
				//数组访问表达式
				leftType = Exp( child );
			}
			else if( strcmp(leftGrandChild->name,"Exp")==0 && leftGrandChild->bro!=NULL && strcmp(leftGrandChild->bro->name,"DOT")==0 ){
				//结构体访问表达式
				leftType = Exp( child );
			}
			else{
				printf( "Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", child->lineno );
			}
			//判断左右是否类型匹配
			SymDes* rightType = Exp( child->bro->bro );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( typeCmp( leftType, rightType )==TRUE )
				return leftType;
			else{
				printf( "Error type 5 at line %d: Type mismatched for assignment.\n", child->lineno );
				return NULL;
			}
		}
		else if( strcmp(child->bro->name,"RELOP")==0 || strcmp(child->bro->name,"AND")==0 || strcmp(child->bro->name,"OR")==0 || strcmp(child->bro->name,"PLUS")==0 || strcmp(child->bro->name,"MINUS")==0 || strcmp(child->bro->name,"STAR")==0 || strcmp(child->bro->name,"DIV")==0 ){
			// Exp: Exp AND Exp
				// | Exp OR Exp
				// | Exp RELOP Exp
				// | Exp PLUS Exp
				// | Exp MINUS Exp
				// | Exp STAR Exp
				// | Exp DIV Exp
			SymDes* leftType = Exp( child );
			SymDes* rightType = Exp( child->bro->bro );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( leftType->kind==BASIC_KIND && rightType->kind==BASIC_KIND && leftType->detail.basic==rightType->detail.basic )
				return leftType;
			else{
				printf( "Error type 7 at line %d: Type mismatched for operands.\n", child->lineno );
				return NULL;
			}
		}
		else if( strcmp(child->bro->name,"LB")==0 ){
			// Exp: Exp LB Exp RB
			SymDes* arraySym = Exp( child );
			if( arraySym==NULL )
				return NULL;
			if( arraySym->kind!=ARRAY_KIND ){
				printf( "Error type 10 at line %d: \"%s\" is not an array.\n", child->lineno, arraySym->name );
				return NULL;
			}
			child = child->bro->bro;
			SymDes* arraySize = Exp( child );
			if( arraySize==NULL )
				return NULL;
			if( arraySize->kind!=BASIC_KIND || arraySize->detail.basic!=INT_TYPE ){
				printf( "Error type 12 at line %d: \"%s\" is not an array.\n", child->lineno , child->child->value );
				return NULL;
			}
			return arraySym->detail.array.formBy;
		}
		else if( strcmp(child->bro->name,"DOT")==0 ){
			// Exp: Exp DOT ID
			SymDes* strucSym = Exp( child );
			if( strucSym==NULL )
				return NULL;
			if( strucSym->kind!=STRUCT_KIND ){
				printf( "Error type 13 at line %d: Illegal use of \".\".\n", child->lineno );
				return NULL;
			}
			SymDes* strucField = strucSym->detail.struc->fieldList;
			child = child->bro->bro;
			while( strucField!=NULL ){
				if( strcmp( strucField->name, child->value) ==0 ){
					return strucField;
				}
				strucField = strucField->nextInList;
			}
			printf( "Error type 14 at line %d: Non-existent field \"%s\".\n", child->lineno, child->value );
			return NULL;
		}
		// can`t reach here
		assert(0);
	}
	else if( strcmp(child->name,"LP")==0 ){
		// Exp: LP Exp RP
		child = child->bro;
		return Exp( child );
	}
	else if( strcmp(child->name,"MINUS")==0 ){
		// Exp: MINUS Exp
		child = child->bro;
		SymDes* right = Exp( child );
		if( right==NULL )
			return NULL;
		if( right->kind!=BASIC_KIND ){
			printf( "Error type 7 at line %d: Operands type mismatched\n", child->lineno );
			return NULL;
		}
		return right;
	}
	else if( strcmp(child->name,"NOT")==0 ){
		// Exp: NOT Exp
		child = child->bro;
		SymDes* right = Exp( child );
		if( right==NULL )
			return NULL;
		if( right->kind!=BASIC_KIND || right->detail.basic!=INT_TYPE ){
			printf( "Error type 7 at line %d: Operands type mismatched\n", child->lineno );
			return NULL;
		}
		return right;
	}
	else if( strcmp(child->name,"INT")==0 ){
		// Exp: INT
		SymDes* type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->kind = BASIC_KIND;
		type->detail.basic = INT_TYPE;
		return type;
	}
	else if( strcmp(child->name,"FLOAT")==0 ){
		// Exp: FLOAT
		SymDes* type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->kind = BASIC_KIND;
		type->detail.basic = FLOAT_TYPE;
		return type;
	}
	else if( strcmp(child->name,"ID")==0 ){
		if( child->bro==NULL ){
			// Exp: ID
			SymDes* sym = findSymDes( child->value );
			if( sym==NULL ){
				printf( "Error type 1 at line %d: Undefined variable \"%s\".\n", child->lineno, child->value );	
				return NULL;
			}
			return sym;
		}
		else{
			// Exp: ID LP (Args) RP
			SymDes* wrongSym = findSymDes( child->value );
			FuncDes* func = findFuncDes( child->value );
			if( wrongSym!=NULL&&func==NULL ){
				printf( "Error type 11 at line %d: \"%s\" is not a function.\n", child->lineno, child->value );
				return NULL;
			}
			if( func==NULL||func->isDef==FALSE ){
				printf( "Error type 2 at line %d: Undefined function \"%s\".\n", child->lineno, child->value );
				return NULL;
			}
			SymDes* paraList = func->paraList;
			child = child->bro->bro;
			if( strcmp(child->name,"RP")==0 ){
				if(paraList!=NULL){
					printf( "Error type 9 at line %d: Function \"%s(", child->lineno, func->name );
					printTypeList( paraList );
					printf( ")\" is not applicable for arguments \"()\".\n" );
				}
			}
			else{
				if( Args( child,paraList )==FALSE ){
					printf( "Error type 9 at line %d: Function \"%s(", child->lineno, func->name );
					printTypeList( paraList );
					printf( ")\" is not applicable for arguments \"(");
					printArgs( child->child );
					printf( ")\".\n" );
				}
			}
			return func->retType;
		}
	}
	return retType;
}

int Args( Node* p, SymDes* paraList ){
	assert( p!=NULL );	
	//printf("%s\n",p->name);
	//Args: Exp (COMMA Args)
	if( p==NULL && paraList==NULL )
		return TRUE;
	else if( p==NULL || paraList==NULL )
		return FALSE;
	Node* child = p->child;
	SymDes* left = Exp(child);
	if( left==NULL )
		return TRUE;
	if( typeCmp( left, paraList )==FALSE )
		return FALSE;
	child = child->bro;
	if( child==NULL && paraList->nextInList==NULL )
		return TRUE;
	else if( child==NULL || paraList->nextInList==NULL )
		return FALSE;
	return Args( child->bro, paraList->nextInList );
}

void printArgs( Node* p ){
	SymDes* type = Exp( p );
	if( type==NULL )
		return;
	printType( type );
	p = p->bro;
	if( p==NULL )
		return;
	else{
		printf(", ");
	}
	printArgs( p->bro->child );
}
