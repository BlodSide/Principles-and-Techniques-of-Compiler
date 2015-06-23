#include "tables.h"
/*
SymDes* symTable[HASH_TABLE_SIZE];
StrucDes* strucTable[HASH_TABLE_SIZE];
*/

FuncDes* funcTable[HASH_TABLE_SIZE];
SymDes* symTable[HASH_TABLE_SIZE];
StrucDes* strucTable[HASH_TABLE_SIZE];
int depth;
SymDes* symStack[STACK_SIZE];
StrucDes* strucStack[STACK_SIZE]; 
/*
* BKDR Hash Function
*/
unsigned int strHash(char* str){
	if( str==NULL )
		return 0;
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;
	while (*str)
		hash = hash * seed + (*str++);
		
	return (hash & 0x7FFF);
}

int symDesCmp(SymDes* a, SymDes* b){
	if( strcmp(a->name, b->name)!=0 )
		return FALSE;
	if( a->kind!=b->kind )
		return FALSE;
	else
		switch( a->kind ){
			case BASIC_KIND:
				if( a->detail.basic!=b->detail.basic )
					return FALSE;
				break;
			case ARRAY_KIND:
				return symDesCmp( a->detail.array.formBy, b->detail.array.formBy );
				break;
			case STRUCT_KIND:
				return symDesListCmp( a->detail.struc->fieldList, b->detail.struc->fieldList );
				break;
			default: return FALSE;
		}
	return TRUE;
}

int symDesListCmp(SymDes* a, SymDes* b){
	while( a!=NULL || b!=NULL ){
		if( a==NULL || b==NULL )
			return FALSE;
		else{
			if( !symDesCmp(a, b) )
				return FALSE;
			else{
				a = a->nextInList;
				b = b->nextInList;
			}
		}
	}
	return TRUE;
}

int typeCmp(SymDes* a, SymDes* b){
	if( a->kind!=b->kind )
		return FALSE;
	else
		switch( a->kind ){
			case BASIC_KIND:
				if( a->detail.basic!=b->detail.basic )
					return FALSE;
				break;
			case ARRAY_KIND:
				return typeCmp( a->detail.array.formBy, b->detail.array.formBy );
				break;
			case STRUCT_KIND:
				return typeListCmp( a->detail.struc->fieldList, b->detail.struc->fieldList );
				break;
			default: return FALSE;
		}
	return TRUE;
}

int typeListCmp(SymDes* a, SymDes* b){
	while( a!=NULL || b!=NULL ){
		if( a==NULL || b==NULL )
			return FALSE;
		else{
			if( !typeCmp(a, b) )
				return FALSE;
			else{
				a = a->nextInList;
				b = b->nextInList;
			}
		}
	}
	return TRUE;
}

void initTable(){
	memset(symTable, 0, HASH_TABLE_SIZE*sizeof(SymDes*));
	memset(strucTable, 0, HASH_TABLE_SIZE*sizeof(StrucDes*));
	memset(funcTable, 0, HASH_TABLE_SIZE*sizeof(FuncDes*));
	depth = 0;
	memset(symStack, 0, STACK_SIZE*sizeof(SymDes*));
	memset(strucStack, 0, STACK_SIZE*sizeof(StrucDes*));
}

int insertSymTable(SymDes* p){
	if( p->name == NULL )
		return -1;
	p->depth = depth;
	unsigned int hash = strHash(p->name);
	if( symTable[hash]==NULL )
		symTable[hash] = p;
	else{
		SymDes* q = symTable[hash];
		while( q != NULL ){
			if( strcmp(p->name, q->name)==0 && p->depth==q->depth )
				return REDEFINE_ERROR;
			q = q->nextInHash;
		}
		p->nextInHash = symTable[hash];
		symTable[hash]->preInHash = p;
		symTable[hash] = p;
	}
	p->nextInStack = symStack[depth];
	symStack[depth] = p;
	return 0;
}

int insertFuncTable(FuncDes* p){
	if( p->name == NULL )
		return -1;
	unsigned int hash = strHash(p->name);
	if( funcTable[hash]==NULL ){
		funcTable[hash] = p;
	}
	else{
		FuncDes* q = funcTable[hash];
		while( q!= NULL ){
			if( strcmp(p->name, q->name) == 0){
				if( q->isDef==TRUE && p->isDef==TRUE )
					return REDEFINE_ERROR;
				else{
					if( typeCmp(p->retType, q->retType)==FALSE || symDesListCmp(p->paraList, q->paraList)==FALSE )
						return INCONSISTENT_DELARATION_ERROR;
					if( p->isDef==TRUE )
						q->isDef = TRUE;
					return 0; //将一个已经声明而未定义的函数进行定义
				}
			}
			q = q->nextInHash;
		}
		p->nextInHash = funcTable[hash];
		funcTable[hash] = p;
	}
	return 0; //将一个未声明未定义的函数进行声明或定义
}

int insertStrucTable( StrucDes* p ){
	if( p->name == NULL )
		return -1;
	p->depth = depth;
	unsigned int hash = strHash(p->name);
	if( strucTable[hash]==NULL )
		strucTable[hash] = p;
	else{
		StrucDes* q = strucTable[hash];
		while( q != NULL ){
			if( strcmp(p->name, q->name)==0 && p->depth==q->depth )
				return REDEFINE_ERROR;
			q = q->nextInHash;
		}
		p->nextInHash = strucTable[hash];
		strucTable[hash]->preInHash = p;
		strucTable[hash] = p;
	}
	p->nextInStack = strucStack[depth];
	strucStack[depth] = p;
	return 0;
}

SymDes* findSymDes(char *name){
	unsigned int hash = strHash(name);
	SymDes* p = symTable[hash];
	while( p!=NULL ){
		if( strcmp(p->name, name)==0 )
			return p;
		else
			p = p->nextInHash;
	}
	return NULL;
}

FuncDes* findFuncDes(char* name){
	unsigned int hash = strHash(name);
	FuncDes* p = funcTable[hash];
	while( p!=NULL ){
		if( strcmp(p->name, name)== 0 )
			return p;
		else
			p = p->nextInHash;
	}
	return NULL;
}

StrucDes* findStrucDes(char* name){
	unsigned int hash = strHash(name);
	StrucDes* p = strucTable[hash];
	while( p!=NULL ){
		if( strcmp(p->name, name)== 0 )
			return p;
		else
			p = p->nextInHash;
	}
	return NULL;
}

void checkFuncDef(){
	int i=0;
	for(; i<HASH_TABLE_SIZE; i++){
		if( funcTable[i]!=NULL ){
			FuncDes* p = funcTable[i];
			while( p!=NULL ){
				if( !p->isDef )
					printf("Error type 18 at line %d: Undefined function \"%s\".\n",p->lineno,p->name);
				p = p->nextInHash;
			}
		}
	}
}

void printType( SymDes* p ){
	switch( p->kind ){
		case BASIC_KIND:
			if( p->detail.basic==INT_TYPE )
				printf("int");
			else if( p->detail.basic==FLOAT_TYPE )
				printf("float");
			break;
		case ARRAY_KIND:
			printType( p->detail.array.formBy );
			printf("[]");
			break;
		case STRUCT_KIND:
			printf("struct %s", p->detail.struc->name);
			break;
	}
}

void printTypeList( SymDes* p ){
	while( p!=NULL ){
		printType( p );
		if(p->nextInList!=NULL)
			printf(", ");
		p = p->nextInList;
	}
}

void tablesPush(){
	depth++;
	if( depth>STACK_SIZE )
		printf("ERROR: Stack boom!\n");
}
void tablesPop(){
	if( depth==0 ){
		printf("ERROR: Stack Pop Exception!\n");
		return;
	}
	SymDes* sym_tem = symStack[depth];
	StrucDes* struc_tem = strucStack[depth];
	while( sym_tem!=NULL ){
		//printf("-----Delete sym: %s\n",sym_tem->name);
		if(sym_tem->preInHash!=NULL)
			sym_tem->preInHash->nextInHash = sym_tem->nextInHash;
		if(sym_tem->nextInHash!=NULL)
			sym_tem->nextInHash->preInHash = sym_tem->preInHash;
		unsigned hash = strHash(sym_tem->name);
		if( symTable[hash]==sym_tem )
			symTable[hash] = sym_tem->nextInHash;
		//SymDes* sym_free = sym_tem;
		sym_tem = sym_tem->nextInStack;
		//free(sym_free);
	}
	symStack[depth] = NULL;
	while( struc_tem!=NULL ){
		if(struc_tem->preInHash!=NULL)
			struc_tem->preInHash->nextInHash = struc_tem->nextInHash;
		if(struc_tem->nextInHash!=NULL)
			struc_tem->nextInHash->preInHash = struc_tem->preInHash;
		unsigned hash = strHash(struc_tem->name);
		if( strucTable[hash]==struc_tem )
			strucTable[hash] = struc_tem->nextInHash;
		//StrucDes* struc_free = struc_tem;
		struc_tem = struc_tem->nextInStack;
		//free(struc_free);
	}
	strucStack[depth] = NULL;
	depth--;
}
