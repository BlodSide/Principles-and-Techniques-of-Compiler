#include "semantic.h"

static char zeroStr[] = "0";

//High-level Definitions
void Program( Node* p ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
	//Program: ExtDefList
	ExtDefList( p->child );
	checkFuncDef();
}

void ExtDefList( Node* p ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
	if( p->child == NULL ){
		return;
	}
	//ExtDefList: ExtDef ExtDefList
	ExtDef( p->child );
	ExtDefList( p->child->bro );
}

void ExtDef( Node* p ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
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
			flag = insertFuncTable(f);
		}
		else{
			//ExtDef: Specifier FunDec CompSt
			f->isDef = TRUE;
			flag = insertFuncTable(f);
			//--------------------------translate-------------
				//FUNCTION f->name :
				Operand* funcOp = malloc(sizeof(Operand));
				memset(funcOp, 0, sizeof(Operand));
				funcOp->kind = FUNC_OP;
				funcOp->u.value = f->name;
				InterCode* funcCode = malloc(sizeof(InterCode));
				memset(funcCode, 0, sizeof(InterCode));
				funcCode->kind = FUNCTION_CODE;
				funcCode->u.singleop.op = funcOp;
				insertCode(funcCode);
				SymDes* paraList = f->paraList;
				//PARAM paraList->name
				while(paraList!=NULL){
					Operand* para = malloc(sizeof(Operand));
					memset(para, 0, sizeof(Operand));
					para->kind = VAR_OP;
					//para->u.value = paraList->name;
					setOpValue(para, paraList->name);
					InterCode* paraCode = malloc(sizeof(InterCode));
					memset(paraCode, 0, sizeof(InterCode));
					paraCode->kind = PARAM_CODE;
					paraCode->u.singleop.op = para;
					insertCode(paraCode);
					paraList = paraList->nextInList;
				}
			//--------------------------translate-------------
			CompSt( child, type );
		}
		tablesPop();
		
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
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	//ExtDecList: VarDec (COMMA ExtDecList)
	SymDes* ret;
	if( child->bro !=NULL ){
		SymDes* typeCpy = malloc( sizeof(SymDes) );
		memcpy( typeCpy, type, sizeof(SymDes) );
		ret = VarDec( child, type, VAR_DEC_IN_EXT);
		ExtDecList( child->bro->bro, typeCpy );
	}
	else
		ret = VarDec( child, type, VAR_DEC_IN_EXT);
	//--------------------------translate---------------------
		if(ret!=NULL && ret->kind==ARRAY_KIND){
			//DEC t_no sizeof(ret)
			Operand* decOp = malloc(sizeof(Operand));
			memset(decOp, 0, sizeof(Operand));
			decOp->kind = TEM_VAR_OP;
			decOp->u.no = temVarNo;
			temVarNo++;
			InterCode* decCode = malloc(sizeof(InterCode));
			memset(decCode, 0, sizeof(InterCode));
			decCode->kind = DEC_CODE;
			decCode->u.dec.op = decOp;
			decCode->u.dec.size = sizeofType(ret);
			insertCode(decCode);
			
			//t_no2 := &t_no
			Operand* decOpAddr = malloc(sizeof(Operand));
			memset(decOpAddr, 0, sizeof(Operand));
			decOpAddr->kind = VAR_OP;
			setOpValue(decOpAddr, ret->name);
			InterCode* addrCode = malloc(sizeof(InterCode));
			memset(addrCode, 0, sizeof(InterCode));
			addrCode->kind = ADDRESS_CODE;
			addrCode->u.assign.left = decOpAddr;
			addrCode->u.assign.right = decOp;
			insertCode(addrCode);
		}		
	//--------------------------translate---------------------
	
}

//Specifiers
SymDes* Specifier( Node* p ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	SymDes* type = NULL;
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
	//printf("%s %d\n",p->name,p->lineno);
	StrucDes* struc;
	SymDes* type = NULL;
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
}

//Declarators
SymDes* VarDec( Node* p, SymDes* type, int context ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
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
		//--------------------------translate-----------------
			//只处理struct
			//array交给上层处理
			if((context==VAR_DEC_IN_EXT || context==VAR_DEC_IN_LOCAL) && type->kind == STRUCT_KIND){
				//DEC decOp sizeof(type)
				Operand* decOp = malloc(sizeof(Operand));
				memset(decOp, 0, sizeof(Operand));
				decOp->kind = TEM_VAR_OP;
				decOp->u.no = temVarNo;
				temVarNo++;
				InterCode* decCode = malloc(sizeof(InterCode));
				memset(decCode, 0, sizeof(InterCode));
				decCode->kind = DEC_CODE;
				decCode->u.dec.op = decOp;
				decCode->u.dec.size = sizeofType(type);
				insertCode(decCode);
				
				//decOpAddr := &decOp
				Operand* decOpAddr = malloc(sizeof(Operand));
				memset(decOpAddr, 0, sizeof(Operand));
				decOpAddr->kind = VAR_OP;
				setOpValue(decOpAddr, type->name);
				InterCode* addrCode = malloc(sizeof(InterCode));
				memset(addrCode, 0, sizeof(InterCode));
				addrCode->kind = ADDRESS_CODE;
				addrCode->u.assign.left = decOpAddr;
				addrCode->u.assign.right = decOp;
				insertCode(addrCode);
			}
		//--------------------------translate-----------------
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
	//printf("%s %d\n",p->name,p->lineno);
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
	//printf("%s %d\n",p->name,p->lineno);
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
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	//ParamDec: Specifier VarDec
	SymDes* type = Specifier(child);
	SymDes* para = VarDec( child->bro, type, VAR_DEC_IN_PARAM );
	return para;
}

/*Statements*/
void CompSt( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child->bro;
	//CompSt: LC DefList StmtList RC
	DefList( child, VAR_DEC_IN_LOCAL );
	child = child->bro;
	StmtList( child, retType );
}

void StmtList( Node* p, SymDes* retType ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
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
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	if( strcmp(child->name,"Exp")==0 ){
		//Stmt: Exp SEMI
		Exp( child, NULL );
		return;
	}
	else if( strcmp(child->name,"CompSt")==0 ){
		//Stmt: CompSt
		tablesPush();
		CompSt( child, retType );
		tablesPop();
		return;
	}
	else if( strcmp(child->name,"RETURN")==0 ){
		//Stmt: RETURN Exp SEMI
		child = child->bro;
		//--------------------------translate-----------------
			//返回用的临时变量
			Operand* place = malloc(sizeof(Operand));
			memset(place, 0, sizeof(Operand));
			place->kind = TEM_VAR_OP;
			place->u.no = temVarNo;
			temVarNo++;
		//--------------------------translate-----------------
		SymDes* ret = Exp( child, place );
		if( retType==NULL && ret==NULL )
			return;
		else if( retType==NULL || ret==NULL || typeCmp( retType, ret )==FALSE ){
			printf( "Error type 8 at line %d: Type mismatched for return.\n", child->lineno );
		}
		//--------------------------translate-----------------
			//RETURN t_no
			InterCode* retCode = malloc(sizeof(InterCode));
			memset(retCode, 0, sizeof(InterCode));
			retCode->kind = RETURN_CODE;
			retCode->u.singleop.op = place;
			insertCode(retCode);
		//--------------------------translate-----------------
		return;
	}
	else if( strcmp(child->name,"WHILE")==0 ){
		//Stmt: WHILE LP Exp RP Stmt
		//--------------------------translate-----------------
			//生成3个label
			
			Operand* label1 = malloc(sizeof(Operand));
			memset(label1, 0, sizeof(Operand));
			label1->kind = LABEL_OP;
			label1->u.no = labelNo;
			labelNo++;
			Operand* label2 = malloc(sizeof(Operand));
			memset(label2, 0, sizeof(Operand));
			label2->kind = LABEL_OP;
			label2->u.no = labelNo;
			labelNo++;
			Operand* label3 = malloc(sizeof(Operand));
			memset(label3, 0, sizeof(Operand));
			label3->kind = LABEL_OP;
			label3->u.no = labelNo;
			labelNo++;
			//LABEL 1 :
			InterCode* label1Code = malloc(sizeof(InterCode));
			memset(label1Code, 0, sizeof(InterCode));
			label1Code->kind = LABEL_CODE;
			label1Code->u.singleop.op = label1;
			insertCode(label1Code);
		//--------------------------translate-----------------
		child = child->bro->bro;
		SymDes* whileExp = ExpOfCondition( child, label2, label3 );
		//--------------------------translate-----------------
			//LABEL 2 :
			InterCode* label2Code = malloc(sizeof(InterCode));
			memset(label2Code, 0, sizeof(InterCode));
			label2Code->kind = LABEL_CODE;
			label2Code->u.singleop.op = label2;
			insertCode(label2Code);
		//--------------------------translate-----------------
		Stmt( child->bro->bro, retType );
		//--------------------------translate-----------------
			//GOTO 1 :
			InterCode* gotoLabel1Code = malloc(sizeof(InterCode));
			memset(gotoLabel1Code, 0, sizeof(InterCode));
			gotoLabel1Code->kind = GOTO_CODE;
			gotoLabel1Code->u.singleop.op = label1;
			insertCode(gotoLabel1Code);
			//LABEL 3 :
			InterCode* label3Code = malloc(sizeof(InterCode));
			memset(label3Code, 0, sizeof(InterCode));
			label3Code->kind = LABEL_CODE;
			label3Code->u.singleop.op = label3;
			insertCode(label3Code);
		//--------------------------translate-----------------
		return;
	}
	else{
		//Stmt: IF LP Exp RP Stmt (ELSE Stmt)
		child = child->bro->bro;
		//--------------------------translate-----------------
			//生成2个label
			Operand* label1 = malloc(sizeof(Operand));
			memset(label1, 0, sizeof(Operand));
			label1->kind = LABEL_OP;
			label1->u.no = labelNo;
			labelNo++;
			Operand* label2 = malloc(sizeof(Operand));
			memset(label2, 0, sizeof(Operand));
			label2->kind = LABEL_OP;
			label2->u.no = labelNo;
			labelNo++;
		//--------------------------translate-----------------
		SymDes* whileExp = ExpOfCondition( child, label1, label2);
		//--------------------------translate-----------------
			//LABEL 1 :
			InterCode* label1Code = malloc(sizeof(InterCode));
			memset(label1Code, 0, sizeof(InterCode));
			label1Code->kind = LABEL_CODE;
			label1Code->u.singleop.op = label1;
			insertCode(label1Code);
		//--------------------------translate-----------------
		child = child->bro->bro;
		Stmt( child, retType );
		if( child->bro!=NULL ){
			//--------------------------translate-------------
				//再生成一个label
				Operand* label3 = malloc(sizeof(Operand));
				memset(label3, 0, sizeof(Operand));
				label3->kind = LABEL_OP;
				label3->u.no = labelNo;
				labelNo++;
				//GOTO 3:
				InterCode* gotoLabel3Code = malloc(sizeof(InterCode));
				memset(gotoLabel3Code, 0, sizeof(InterCode));
				gotoLabel3Code->kind = GOTO_CODE;
				gotoLabel3Code->u.singleop.op = label3;
				insertCode(gotoLabel3Code);
				//LABEL 2 :
				InterCode* label2Code = malloc(sizeof(InterCode));
				memset(label2Code, 0, sizeof(InterCode));
				label2Code->kind = LABEL_CODE;
				label2Code->u.singleop.op = label2;
				insertCode(label2Code);
			//--------------------------translate-------------
			Stmt( child->bro->bro, retType );
			//--------------------------translate-------------
				//LABEL 3 :
				InterCode* label3Code = malloc(sizeof(InterCode));
				memset(label3Code, 0, sizeof(InterCode));
				label3Code->kind = LABEL_CODE;
				label3Code->u.singleop.op = label3;
				insertCode(label3Code);
			//--------------------------translate-------------
		}
		else{
			//--------------------------translate--------------
				//LABEL 2 :
				InterCode* label2Code = malloc(sizeof(InterCode));
				memset(label2Code, 0, sizeof(InterCode));
				label2Code->kind = LABEL_CODE;
				label2Code->u.singleop.op = label2;
				insertCode(label2Code);
			//--------------------------translate--------------
		}
		return;
	}
}

/*Local Definitions*/
SymDes* DefList( Node* p, int context ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
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
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	//Def: Specifier DecList SEMI
	SymDes* type = Specifier( child );
	return DecList( child->bro, type, context );
}

SymDes* DecList( Node* p, SymDes* type, int context ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);
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
	//printf("%s %d\n",p->name,p->lineno);
	Node* child = p->child;
	//Dec: VarDec (ASSIGNOP Exp)
	SymDes* left = VarDec( child, type, context );
	
	//--------------------------translate---------------------
		if(context!=VAR_DEC_IN_STRUCT && left->kind==ARRAY_KIND){
			//DEC t_no sizeof(left)
			Operand* decOp = malloc(sizeof(Operand));
			memset(decOp, 0, sizeof(Operand));
			decOp->kind = TEM_VAR_OP;
			decOp->u.no = temVarNo;
			temVarNo++;
			InterCode* decCode = malloc(sizeof(InterCode));
			memset(decCode, 0, sizeof(InterCode));
			decCode->kind = DEC_CODE;
			decCode->u.dec.op = decOp;
			decCode->u.dec.size = sizeofType(left);
			insertCode(decCode);
			
			//t_no2 := &t_no
			Operand* decOpAddr = malloc(sizeof(Operand));
			memset(decOpAddr, 0, sizeof(Operand));
			decOpAddr->kind = VAR_OP;
			setOpValue(decOpAddr, left->name);
			InterCode* addrCode = malloc(sizeof(InterCode));
			memset(addrCode, 0, sizeof(InterCode));
			addrCode->kind = ADDRESS_CODE;
			addrCode->u.assign.left = decOpAddr;
			addrCode->u.assign.right = decOp;
			insertCode(addrCode);
		}
	//--------------------------translate---------------------
	
	child = child->bro;
	if( child!=NULL ){
		if( context==VAR_DEC_IN_STRUCT ){
			printf( "Error type 15 at line %d: be initialized field \"%s\".\n", child->lineno, left->name );
			return left;
		}
		//--------------------------translate-----------------
			Operand* place = malloc(sizeof(Operand));
			memset(place, 0, sizeof(Operand));
			place->kind = VAR_OP;
			setOpValue(place, left->name);
			char* placeValue = place->u.value;
		//--------------------------translate-----------------
		child = child->bro;
		SymDes* right = Exp(child, place);
		if( left!=NULL && right!=NULL && typeCmp( left, right )==FALSE ){
			printf( "Error type 5 at line %d: Type mismatched for assignment.\n", child->lineno );
		}
		//--------------------------translate-----------------
			if(place->kind!=VAR_OP || placeValue != place->u.value){
				//只有place被改 才说明需要生成赋值语句
				//left->name := Exp()->name
				Operand* leftOp = malloc(sizeof(Operand));
				memset(leftOp, 0, sizeof(Operand));
				leftOp->kind = VAR_OP;
				setOpValue(leftOp, left->name);
				InterCode* assignCode = malloc(sizeof(InterCode));
				memset(assignCode, 0, sizeof(InterCode));
				assignCode->kind = ASSIGN_CODE;
				assignCode->u.assign.left = leftOp;
				assignCode->u.assign.right = place;
				insertCode(assignCode);
			}
		//--------------------------translate-----------------
	}
	return left;
}

/*Expressions*/
SymDes* Exp( Node* p, Operand* place ){
	assert( p!=NULL );
	//printf("%s %d\n",p->name,p->lineno);	
	Node* child = p->child;
	SymDes* retType=NULL;
	if( strcmp(child->name,"Exp")==0 ){
		if( strcmp(child->bro->name,"ASSIGNOP")==0 ){
			//Exp: Exp ASSIGNOP Exp
			Node* leftGrandChild = child->child;
			// 以下判断左表达式是否合法
			// 左表达式只可能是普通变量、数组访问表达式、结构体访问表达式
			SymDes* leftType = NULL;
			//--------------------------translate-------------
				Operand* leftOp = malloc(sizeof(Operand));
				memset(leftOp, 0, sizeof(Operand));
				leftOp->kind = TEM_VAR_OP;
				leftOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------
			if( strcmp(leftGrandChild->name,"ID")==0 && leftGrandChild->bro==NULL ){
				//基本变量
				leftType = Exp( child, leftOp );
			}
			else if( strcmp(leftGrandChild->name,"Exp")==0 && leftGrandChild->bro!=NULL && strcmp(leftGrandChild->bro->name,"LB")==0 ){
				//数组访问表达式
				leftType = Exp( child, leftOp);
			}
			else if( strcmp(leftGrandChild->name,"Exp")==0 && leftGrandChild->bro!=NULL && strcmp(leftGrandChild->bro->name,"DOT")==0 ){
				//结构体访问表达式
				leftType = Exp( child, leftOp );
			}
			else{
				printf( "Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", child->lineno );
			}
			//--------------------------translate-------------
				Operand* rightOp = malloc(sizeof(Operand));
				memset(rightOp, 0, sizeof(Operand));
				rightOp->kind = TEM_VAR_OP;
				rightOp->u.no = temVarNo;
				int rightOpNo = rightOp->u.no;
				temVarNo++;
			//--------------------------translate-------------
			SymDes* rightType = Exp( child->bro->bro, rightOp );
			//判断左右是否类型匹配
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( typeCmp( leftType, rightType )==TRUE ){
				//--------------------------translate---------
					if( !(rightOp->kind==TEM_VAR_OP && rightOp->u.no==rightOpNo && (leftOp->kind==TEM_VAR_OP || leftOp->kind==VAR_OP)) ){
						//rightOp被改 说明需要生成赋值语句
						// leftOp := rightOp
						InterCode* assignCode1 = malloc(sizeof(InterCode));
						memset(assignCode1, 0, sizeof(InterCode));
						assignCode1->kind = ASSIGN_CODE;
						assignCode1->u.assign.left = leftOp;
						assignCode1->u.assign.right = rightOp;
						insertCode(assignCode1);
					}
					else{
						//否则将rightOp替换为leftOp即可
						memcpy(rightOp, leftOp, sizeof(Operand));
					}
					if(place!=NULL){
						//place := rightOp
						InterCode* assignCode2 = malloc(sizeof(InterCode));
						memset(assignCode2, 0, sizeof(InterCode));
						assignCode2->kind = ASSIGN_CODE;
						assignCode2->u.assign.left = place;
						assignCode2->u.assign.right = rightOp;
						insertCode(assignCode2);
					}
				//--------------------------translate---------
				return leftType;
			}
			else{
				printf( "Error type 5 at line %d: Type mismatched for assignment.\n", child->lineno );
				return NULL;
			}
		}
		else if( strcmp(child->bro->name,"PLUS")==0 || strcmp(child->bro->name,"MINUS")==0 || strcmp(child->bro->name,"STAR")==0 || strcmp(child->bro->name,"DIV")==0 ){
			// Exp: Exp PLUS Exp
			// | Exp MINUS Exp
			// | Exp STAR Exp
			// | Exp DIV Exp
			//--------------------------translate-------------
				Operand* leftOp = malloc(sizeof(Operand));
				memset(leftOp, 0, sizeof(Operand));
				leftOp->kind = TEM_VAR_OP;
				leftOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------
			SymDes* leftType = Exp( child, leftOp );
			//--------------------------translate-------------
				Operand* rightOp = malloc(sizeof(Operand));
				memset(rightOp, 0, sizeof(Operand));
				rightOp->kind = TEM_VAR_OP;
				rightOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------
			SymDes* rightType = Exp( child->bro->bro, rightOp );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( leftType->kind==BASIC_KIND && rightType->kind==BASIC_KIND && leftType->detail.basic==rightType->detail.basic ){
				//--------------------------translate---------
				if(place!=NULL){
					InterCode* calcCode = malloc(sizeof(InterCode));
					memset(calcCode, 0, sizeof(InterCode));
					if(strcmp(child->bro->name,"PLUS")==0)
						calcCode->kind=ADD_CODE;
					else if(strcmp(child->bro->name,"MINUS")==0)
						calcCode->kind=SUB_CODE;
					else if(strcmp(child->bro->name,"STAR")==0)
						calcCode->kind=MUL_CODE;
					else if(strcmp(child->bro->name,"DIV")==0)
						calcCode->kind=DIV_CODE;
					else
						assert(0);
					calcCode->u.doubleop.result = place;
					calcCode->u.doubleop.op1 = leftOp;
					calcCode->u.doubleop.op2 = rightOp;
					insertCode(calcCode);
				}
				//--------------------------translate---------
				return leftType;
			}
			else{
				printf( "Error type 7 at line %d: Type mismatched for operands.\n", child->lineno );
				return NULL;
			}
		}
		else if(strcmp(child->bro->name,"RELOP")==0 || strcmp(child->bro->name,"AND")==0 || strcmp(child->bro->name,"OR")==0){
			assert(0);
		}
		else if( strcmp(child->bro->name,"LB")==0 ){
			// Exp: Exp LB Exp RB
			//--------------------------translate-------------
				//用于存储基地址
				Operand* baseOp = malloc(sizeof(Operand));
				memset(baseOp, 0, sizeof(Operand));
				baseOp->kind = TEM_VAR_OP;
				baseOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------
			SymDes* arraySym = Exp( child, baseOp );
			if( arraySym==NULL )
				return NULL;
			if( arraySym->kind!=ARRAY_KIND ){
				printf( "Error type 10 at line %d: \"%s\" is not an array.\n", child->lineno, arraySym->name );
				return NULL;
			}
			//--------------------------translate-------------
				int subscipt = 1;
				if(strcmp(child->bro->bro->child->name, "INT")==0)
					subscipt = atoi(child->bro->bro->child->value);
				Operand* subscriptOp=NULL;
				if(subscipt!=0){
					//用于存储数组下标
					subscriptOp = malloc(sizeof(Operand));
					memset(subscriptOp, 0, sizeof(Operand));
					subscriptOp->kind = TEM_VAR_OP;
					subscriptOp->u.no = temVarNo;
					temVarNo++;
				}
			//--------------------------translate-------------
			child = child->bro->bro;
			SymDes* arraySize = Exp( child, subscriptOp );
			if( arraySize==NULL )
				return NULL;
			if( arraySize->kind!=BASIC_KIND || arraySize->detail.basic!=INT_TYPE ){
				printf( "Error type 12 at line %d: \"%s\" is not an array.\n", child->lineno , child->child->value );
				return NULL;
			}
			//--------------------------translate-------------
				//用于存储计算出的偏移量
				Operand* offsetOp = malloc(sizeof(Operand));
				memset(offsetOp, 0, sizeof(Operand));
				offsetOp->kind = TEM_VAR_OP;
				offsetOp->u.no = temVarNo;
				temVarNo++;				
				if(subscipt!=0){
				//用于存储宽度
					Operand* wideOp = malloc(sizeof(Operand));
					memset(wideOp, 0, sizeof(Operand));
					wideOp->kind = CONST_OP;
					char* wideStr = malloc(16*sizeof(char));
					int wide = sizeofType(arraySym->detail.array.formBy);
					sprintf(wideStr, "%d", wide);
					wideOp->u.value = wideStr;
				
					//offsetOp := subscriptOp MUL wideOp
					InterCode* offsetCode = malloc(sizeof(InterCode));
					memset(offsetCode, 0, sizeof(InterCode));
					offsetCode->kind = MUL_CODE;
					offsetCode->u.doubleop.result = offsetOp;
					offsetCode->u.doubleop.op1 = subscriptOp;
					offsetCode->u.doubleop.op2 = wideOp;
					insertCode(offsetCode);
					
					// 地址 := baseOp ADD offsetOp
					InterCode* addrCode = malloc(sizeof(InterCode));
					memset(addrCode, 0, sizeof(InterCode));
					addrCode->kind = ADD_CODE;
					addrCode->u.doubleop.op1 = baseOp;
					addrCode->u.doubleop.op2 = offsetOp;
					if(arraySym->detail.array.formBy->kind==BASIC_KIND){
						//如果下一层是BASIC 则place是地址指向的位置
						Operand* temAddrOp = malloc(sizeof(Operand));
						memset(temAddrOp, 0, sizeof(Operand));
						temAddrOp->kind = TEM_VAR_OP;
						temAddrOp->u.no = temVarNo;
						temVarNo++;
					
						addrCode->u.doubleop.result = temAddrOp;
						place->kind = ADDR_OP;
						place->u.addr = temAddrOp;
					}
					else{
						//如果下一层仍然是数组 则place是地址
						addrCode->u.doubleop.result = place;
					}
					insertCode(addrCode);
				}
				else{
					// 地址 := baseOp
					InterCode* addrCode = malloc(sizeof(InterCode));
					memset(addrCode, 0, sizeof(InterCode));
					addrCode->kind = ASSIGN_CODE;
					addrCode->u.assign.right = baseOp;
					if(arraySym->detail.array.formBy->kind==BASIC_KIND){
						//如果下一层是BASIC 则place是地址指向的位置
						Operand* temAddrOp = malloc(sizeof(Operand));
						memset(temAddrOp, 0, sizeof(Operand));
						temAddrOp->kind = TEM_VAR_OP;
						temAddrOp->u.no = temVarNo;
						temVarNo++;
					
						addrCode->u.assign.left = temAddrOp;
						place->kind = ADDR_OP;
						place->u.addr = temAddrOp;
					}
					else{
						//如果下一层仍然是数组 则place是地址
						addrCode->u.assign.left = place;
					}
					insertCode(addrCode);
				}
			//--------------------------translate-------------
			return arraySym->detail.array.formBy;
		}
		else if( strcmp(child->bro->name,"DOT")==0 ){
			// Exp: Exp DOT ID
			//--------------------------translate-------------
				Operand* strucVarOp = malloc(sizeof(Operand));
				memset(strucVarOp, 0, sizeof(Operand));
				strucVarOp->kind = TEM_VAR_OP;
				strucVarOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate-------------
			SymDes* strucSym = Exp( child, strucVarOp );
			if( strucSym==NULL )
				return NULL;
			if( strucSym->kind!=STRUCT_KIND ){
				printf( "Error type 13 at line %d: Illegal use of \".\".\n", child->lineno );
				return NULL;
			}
			assert(strucSym->kind == STRUCT_KIND);
			SymDes* strucField = strucSym->detail.struc->fieldList;
			child = child->bro->bro;
			assert(strcmp( "ID", child->name) ==0);
			int offset = 0;
			while( strucField!=NULL ){
				if( strcmp( strucField->name, child->value) ==0 ){
					//--------------------------translate-----
					if(offset==0){
						if(place!=NULL){
							if(strucField->kind==BASIC_KIND){
								place->kind = ADDR_OP;
								place->u.addr = strucVarOp;
							}
							else{
								memcpy(place, strucVarOp, sizeof(Operand));
							}
						}
					}
					else{
						//用于存储偏移量
						Operand* offsetOp = malloc(sizeof(Operand));
						memset(offsetOp, 0, sizeof(Operand));
						offsetOp->kind = CONST_OP;
						char* offsetStr = malloc(16*sizeof(char));
						sprintf(offsetStr, "%d", offset);
						offsetOp->u.value = offsetStr;
						// 地址 := strucVarOp ADD offsetOp
						InterCode* addrCode = malloc(sizeof(InterCode));
						memset(addrCode, 0, sizeof(InterCode));
						addrCode->kind = ADD_CODE;
						addrCode->u.doubleop.op1 = strucVarOp;
						addrCode->u.doubleop.op2 = offsetOp;
						if(strucField->kind==BASIC_KIND){
							//如果下一层是BASIC 则place是地址指向的位置
							Operand* temAddrOp = malloc(sizeof(Operand));
							memset(temAddrOp, 0, sizeof(Operand));
							temAddrOp->kind = TEM_VAR_OP;
							temAddrOp->u.no = temVarNo;
							temVarNo++;
							
							addrCode->u.doubleop.result = temAddrOp;
							place->kind = ADDR_OP;
							place->u.addr = temAddrOp;
						}
						else{
							//如果下一层仍然是struct 则place是地址
							addrCode->u.doubleop.result = place;
						}
						insertCode(addrCode);
					}
					//--------------------------translate-----
					return strucField;
				}
				offset += sizeofType(strucField);
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
		return Exp( child, place );
	}
	else if( strcmp(child->name,"MINUS")==0 ){
		// Exp: MINUS Exp
		child = child->bro;
		//--------------------------translate----------------
			Operand* rightOp = malloc(sizeof(Operand));
			memset(rightOp, 0, sizeof(Operand));
			rightOp->kind = TEM_VAR_OP;
			rightOp->u.no = temVarNo;
			int rightOpNo = rightOp->u.no;
			temVarNo++;
		//--------------------------translate-----------------
		SymDes* right = Exp( child, rightOp );
		if( right==NULL )
			return NULL;
		if( right->kind!=BASIC_KIND ){
			printf( "Error type 7 at line %d: Operands type mismatched\n", child->lineno );
			return NULL;
		}
		//--------------------------translate-----------------
			Operand* zeroOp =  malloc(sizeof(Operand));
			memset(zeroOp, 0, sizeof(Operand));
			zeroOp->kind = CONST_OP;
			zeroOp->u.value = zeroStr;
			// place := #0 - Exp
			if(place!=NULL){
				InterCode* minusCode = malloc(sizeof(InterCode));
				memset(minusCode, 0, sizeof(InterCode));
				minusCode->kind = SUB_CODE;
				minusCode->u.doubleop.result = place;
				minusCode->u.doubleop.op1 = zeroOp;
				minusCode->u.doubleop.op2 = rightOp;
				insertCode(minusCode);
			}
		//--------------------------translate-----------------
		return right;
	}
	else if( strcmp(child->name,"NOT")==0 ){
		assert(0);
	}
	else if( strcmp(child->name,"INT")==0 ){
		// Exp: INT
		SymDes* type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->kind = BASIC_KIND;
		type->detail.basic = INT_TYPE;
		//--------------------------translate-----------------
			if(place!=NULL){
				place->kind = CONST_OP;
				place->u.value = child->value;
			}
		//--------------------------translate-----------------
		return type;
	}
	else if( strcmp(child->name,"FLOAT")==0 ){
		// Exp: FLOAT
		SymDes* type = malloc( sizeof(SymDes) );
		memset( type, 0, sizeof(SymDes) );
		type->kind = BASIC_KIND;
		type->detail.basic = FLOAT_TYPE;
		//--------------------------translate-----------------
			if(place!=NULL){
				place->kind = CONST_OP;
				place->u.value = child->value;
			}
		//--------------------------translate-----------------
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
			//--------------------------translate------------
				if(place!=NULL){
					place->kind = VAR_OP;
					// place->u.value = child->value;
					setOpValue(place, child->value);
				}
			//--------------------------translate------------
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
				else{
					//--------------------------translate----
					if(strcmp(func->name,"read")==0){
						if(place!=NULL){
							InterCode* funcCode = malloc(sizeof(InterCode));
							memset(funcCode, 0, sizeof(InterCode));
							funcCode->kind = READ_CODE;
							funcCode->u.singleop.op = place;
							insertCode(funcCode);
						}
					}
					else{
						Operand* funcOp = malloc(sizeof(Operand));
						memset(funcOp, 0, sizeof(Operand));
						funcOp->kind = FUNC_OP;
						funcOp->u.value = func->name;
						if(place!=NULL){
							InterCode* funcCode = malloc(sizeof(InterCode));
							memset(funcCode, 0, sizeof(InterCode));
							funcCode->kind = CALL_CODE;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
						else{
							Operand* uselessOp = malloc(sizeof(Operand));
							memset(uselessOp, 0, sizeof(Operand));
							uselessOp->kind = TEM_VAR_OP;
							uselessOp->u.no = temVarNo;
							temVarNo++;
							
							InterCode* funcCode = malloc(sizeof(InterCode));
							memset(funcCode, 0, sizeof(InterCode));
							funcCode->kind = CALL_CODE;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
					}
					//--------------------------translate----
				}
				
			}
			else{
				//--------------------------translate--------
					Operand* argsListHead =  NULL;
				//--------------------------translate--------
				if( Args( child, paraList, &argsListHead )==FALSE ){
					printf( "Error type 9 at line %d: Function \"%s(", child->lineno, func->name );
					printTypeList( paraList );
					printf( ")\" is not applicable for arguments \"(");
					printArgs( child->child );
					printf( ")\".\n" );
				}
				else{
					//--------------------------translate----
					if(strcmp(func->name,"write")==0){
						InterCode* funcCode = malloc(sizeof(InterCode));
						memset(funcCode, 0, sizeof(InterCode));
						funcCode->kind = WRITE_CODE;
						assert(argsListHead!=NULL);
						funcCode->u.singleop.op = argsListHead;
						insertCode(funcCode);
					}
					else{
						Operand* argsP = argsListHead;
						while(argsP!=NULL){
							// PARAM argsP
							//直接按照队列方向即从右向左入栈
							InterCode* argCode = malloc(sizeof(InterCode));
							memset(argCode, 0, sizeof(InterCode));
							argCode->kind = ARG_CODE;
							argCode->u.singleop.op = argsP;
							insertCode(argCode);
							argsP = argsP->nextArg;
						}
						Operand* funcOp = malloc(sizeof(Operand));
						memset(funcOp, 0, sizeof(Operand));
						funcOp->kind = FUNC_OP;
						funcOp->u.value = func->name;
						if(place!=NULL){
							InterCode* funcCode = malloc(sizeof(InterCode));
							memset(funcCode, 0, sizeof(InterCode));
							funcCode->kind = CALL_CODE;
							funcCode->u.assign.left = place;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
						else{
							Operand* uselessOp = malloc(sizeof(Operand));
							memset(uselessOp, 0, sizeof(Operand));
							uselessOp->kind = TEM_VAR_OP;
							uselessOp->u.no = temVarNo;
							temVarNo++;
							
							InterCode* funcCode = malloc(sizeof(InterCode));
							memset(funcCode, 0, sizeof(InterCode));
							funcCode->kind = CALL_CODE;
							funcCode->u.assign.left = uselessOp;
							funcCode->u.assign.right = funcOp;
							insertCode(funcCode);
						}
					}
					//--------------------------translate----
				}
			}
			return func->retType;
		}
	}
	return retType;
}

SymDes* ExpOfCondition(Node* p, Operand* trueLabel, Operand* falseLabel ){
	assert( p!=NULL );
	Node *child = p->child;
	if( strcmp(child->name,"Exp")==0 ){
		if( strcmp(child->bro->name,"RELOP")==0 ){
			// Exp: Exp RELOP Exp
			//--------------------------translate------------
				Operand* leftOp = malloc(sizeof(Operand));
				memset(leftOp, 0, sizeof(Operand));
				leftOp->kind = TEM_VAR_OP;
				leftOp->u.no = temVarNo;
				temVarNo++;
			//--------------------------translate------------
			SymDes* leftType = Exp( child, leftOp );
			//--------------------------translate------------
				Operand* rightOp = malloc(sizeof(Operand));
				memset(rightOp, 0, sizeof(Operand));
				rightOp->kind = TEM_VAR_OP;
				rightOp->u.no = temVarNo;
				int rightOpNo = rightOp->u.no;
				temVarNo++;
			//--------------------------translate------------
			SymDes* rightType = Exp( child->bro->bro, rightOp );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( leftType->kind==BASIC_KIND && rightType->kind==BASIC_KIND && leftType->detail.basic==rightType->detail.basic ){
				//--------------------------translate--------
					//IF leftOp RELOP rightOp GOTO trueLabel
					InterCode* ifgotoCode = malloc(sizeof(InterCode));
					memset(ifgotoCode, 0, sizeof(InterCode));
					ifgotoCode->kind = IF_GOTO_CODE;
					ifgotoCode->u.tribleop.x = leftOp;
					ifgotoCode->u.tribleop.y = rightOp;
					ifgotoCode->u.tribleop.relop = child->bro->value;
					ifgotoCode->u.tribleop.gotoLabel = trueLabel;
					insertCode(ifgotoCode);
					// GOTO falseLabel
					InterCode* gotoFalseLabel = malloc(sizeof(InterCode));
					memset(gotoFalseLabel, 0, sizeof(InterCode));
					gotoFalseLabel->kind = GOTO_CODE;
					gotoFalseLabel->u.singleop.op = falseLabel;
					insertCode(gotoFalseLabel);
				//--------------------------translate--------
				return leftType;
			}
			else{
				printf( "Error type 7 at line %d: Type mismatched for operands.\n", child->lineno );
				return NULL;
			}
		}
		else if( strcmp(child->bro->name,"AND")==0 ){
			// Exp: Exp AND Exp
			//--------------------------translate------------
				Operand* leftTrueLabel = malloc(sizeof(Operand));
				memset(leftTrueLabel, 0, sizeof(Operand));
				leftTrueLabel->kind = LABEL_OP;
				leftTrueLabel->u.no = labelNo;
				labelNo++;
			//--------------------------translate------------
			SymDes* leftType = ExpOfCondition( child, leftTrueLabel, falseLabel );
			//--------------------------translate------------
				InterCode* leftTrueLabelCode = malloc(sizeof(InterCode));
				memset(leftTrueLabelCode, 0, sizeof(InterCode));
				leftTrueLabelCode->kind = LABEL_CODE;
				leftTrueLabelCode->u.singleop.op = leftTrueLabel;
				insertCode(leftTrueLabelCode);
			//--------------------------translate------------
			SymDes* rightType = ExpOfCondition( child->bro->bro, trueLabel, falseLabel );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( leftType->kind==BASIC_KIND && rightType->kind==BASIC_KIND && leftType->detail.basic==rightType->detail.basic )
				return leftType;
			else{
				printf( "Error type 7 at line %d: Type mismatched for operands.\n", child->lineno );
				return NULL;
			}
		}
		else if( strcmp(child->bro->name,"OR")==0 ){
			// Exp: Exp OR Exp
			//--------------------------translate------------
				Operand* leftFalseLabel = malloc(sizeof(Operand));
				memset(leftFalseLabel, 0, sizeof(Operand));
				leftFalseLabel->kind = LABEL_OP;
				leftFalseLabel->u.no = labelNo;
				labelNo++;
			//--------------------------translate------------
			SymDes* leftType = ExpOfCondition( child, trueLabel, leftFalseLabel );
			//--------------------------translate------------
				InterCode* leftFalseLabelCode = malloc(sizeof(InterCode));
				memset(leftFalseLabelCode, 0, sizeof(InterCode));
				leftFalseLabelCode->kind = LABEL_CODE;
				leftFalseLabelCode->u.singleop.op = leftFalseLabel;
				insertCode(leftFalseLabelCode);
			//--------------------------translate------------
			SymDes* rightType = ExpOfCondition( child->bro->bro, trueLabel, falseLabel );
			if( leftType==NULL || rightType==NULL )
				return NULL;
			if( leftType->kind==BASIC_KIND && rightType->kind==BASIC_KIND && leftType->detail.basic==rightType->detail.basic )
				return leftType;
			else{
				printf( "Error type 7 at line %d: Type mismatched for operands.\n", child->lineno );
				return NULL;
			}
		}
		assert(0);
	}
	else{
		// Exp: NOT Exp
		SymDes* right = ExpOfCondition( child->bro, falseLabel, trueLabel );
		if( right==NULL )
			return NULL;
		if( right->kind!=BASIC_KIND || right->detail.basic!=INT_TYPE ){
			printf( "Error type 7 at line %d: Operands type mismatched\n", child->bro->lineno );
			return NULL;
		}
		return right;
	}
}

int Args( Node* p, SymDes* paraList, Operand** argsListHead ){
	assert( p!=NULL );	
	//printf("%s %d\n",p->name,p->lineno);
	//Args: Exp (COMMA Args)
	if( p==NULL && paraList==NULL )
		return TRUE;
	else if( p==NULL || paraList==NULL )
		return FALSE;
	//--------------------------translate--------------------
		Operand* argOp = malloc(sizeof(Operand));
		memset(argOp, 0, sizeof(Operand));
		argOp->kind = TEM_VAR_OP;
		argOp->u.no = temVarNo;
		temVarNo++;
	//--------------------------translate--------------------
	Node* child = p->child;
	SymDes* left = Exp(child, argOp);
	//--------------------------translate--------------------
		//参数顺序为 从右往左 即第一个参数在队列最末端 方便入栈
		argOp->nextArg = *argsListHead;
		*argsListHead = argOp;
	//--------------------------translate--------------------
	if( left==NULL )
		return TRUE;
	if( typeCmp( left, paraList )==FALSE )
		return FALSE;
	child = child->bro;
	if( child==NULL && paraList->nextInList==NULL )
		return TRUE;
	else if( child==NULL || paraList->nextInList==NULL )
		return FALSE;
	return Args( child->bro, paraList->nextInList , argsListHead);
}

void printArgs( Node* p ){
	SymDes* type = Exp( p, NULL );
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
