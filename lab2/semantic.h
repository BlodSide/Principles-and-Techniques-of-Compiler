#ifndef _SEMANTIC_H_
#define _SEMANTIC_H_

#include "tables.h"
#include "node.h"
#include <assert.h>

#define VAR_DEC_IN_LOCAL 0
#define VAR_DEC_IN_EXT 1
#define VAR_DEC_IN_STRUCT 2
#define VAR_DEC_IN_PARAM 3

void Program( Node* p );
void ExtDefList( Node* p );
void ExtDef( Node* p );
void ExtDecList( Node* p, SymDes* type );

SymDes* Specifier( Node* p );
SymDes* StructSpecifier( Node* p );

SymDes* VarDec( Node* p, SymDes* type, int context );
FuncDes* FunDec( Node* p, SymDes* retType );
SymDes*  VarList( Node* p );
SymDes* ParamDec( Node* p );

void CompSt( Node* p, SymDes* retType );
void StmtList( Node* p, SymDes* retType );
void Stmt( Node* p, SymDes* retType );

SymDes* DefList( Node* p, int context );
SymDes* Def( Node* p, int context );
SymDes* DecList( Node* p, SymDes* type, int context );
SymDes* Dec( Node* p, SymDes* type, int context );

SymDes* Exp( Node* p );
int Args( Node* p, SymDes* para );

void printArgs( Node* p );
#endif
