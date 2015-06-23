#ifndef _OBJCODE_H_
#define _OBJCODE_H_

#include "interCode.h"

#define REGNUM 18
#define FSIZE 8	

typedef struct VarDes_t{
	Operand* op;
	int regNo;
	int offset;
	struct VarDes_t* next;
} VarDes;

typedef struct RegDes_t{
	char name[3];
	VarDes* var;
	int old;
} RegDes;

void clearVarList();
void addVarToList(VarDes* p);
void stVar(VarDes* var, FILE* fp);
void ldVar(VarDes* var, FILE* fp);
void initAllRegDes();
void rstAllReg();
void freeOneReg(int regNo);
int allocateRegForOp(Operand* op, FILE* fp);
int getReg(FILE* fp);
void fwriteAllObjCode(char* fileName);
void fwriteOneObjCode(InterCode* ir, FILE* fp);

#endif
