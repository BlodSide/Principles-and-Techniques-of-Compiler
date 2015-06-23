#ifndef _INTERCODE_H_
#define _INTERCODE_H_

#include"tables.h"
#include<assert.h>

#define TRUE 1
#define FALSE 0

#define VAR_OP 			1
#define CONST_OP		2
#define ADDR_OP 		3
#define TEM_VAR_OP 		4
#define LABEL_OP		5
#define FUNC_OP			6

#define LABEL_CODE		1
#define FUNCTION_CODE		2
#define ASSIGN_CODE 		3
#define ADD_CODE		4
#define SUB_CODE		5
#define MUL_CODE		6
#define DIV_CODE		7
#define ADDRESS_CODE		8
#define GOTO_CODE		9
#define IF_GOTO_CODE		10
#define RETURN_CODE		11
#define DEC_CODE		12
#define ARG_CODE		13
#define CALL_CODE		14
#define PARAM_CODE		15
#define READ_CODE		16
#define WRITE_CODE		17

typedef struct Operand_t
{
	//VAR_OP CONST_OP ADDR_OP TEM_VAR_OP LABEL_OP FUNC_OP 
	int kind;
	union{
		//TEM_VAR_OP LABEL_OP
		int no;
		//VAR_OP CONST_OP FUNC_OP
		char* value;
		//ADDR_OP
		struct Operand_t* addr;
	}u;
	//在参数列表中的时候 指向下一个参数
	struct Operand_t* nextArg;
} Operand;

typedef struct InterCode_t
{
	//LABEL_CODE FUNCTION_CODE ASSIGN_CODE ADD_CODE SUB_CODE MUL_CODE DIV_CODE ADDRESS_CODE GOTO_CODE IF_GOTO_CODE RETURN_CODE DEC_CODE ARG_CODE CALL_CODE PARAM_CODE READ_CODE WRITE_CODE
	int kind;
	union{
		//useless是将除了DEC_CODE以外的CODE的左值和左值均对齐 右值和右值均对齐
		//ASSIGN_CODE ADDRESS_CODE CALL_CODE
		struct{ 
			Operand* left;
			Operand* right;
			Operand* useless1;
		} assign;
		
		//LABEL_CODE FUNCTION_CODE GOTO_CODE RETURN_CODE ARG_CODE PARAM_CODE READ_CODE WRITE_CODE
		struct{	
			Operand* useless2;
			Operand* op;
			Operand* useless3;
		} singleop;
		
		//ADD_CODE SUB_CODE MUL_CODE DIV_CODE
		struct{ 
			Operand* result;
			Operand* op1;
			Operand* op2; 
		} doubleop;	
		
		//IF_GOTO_CODE 
		struct{ 
			Operand* gotoLabel;
			Operand* x;
			Operand* y;
			char* relop;
		} tribleop;			
		
		//DEC_CODE 
		struct{ 
			Operand* op;
			int size; 
		} dec; 
	}u;
	struct InterCode_t* preCode;
	struct InterCode_t* nextCode;
} InterCode;

extern int temVarNo;
extern int labelNo;
extern InterCode* codeHead;

void insertCode(InterCode* p);
void deleteCode(InterCode* p);
void fwriteOp(Operand* p, FILE* fp);
void fwriteAllInterCode(char* fileName);

void printOp(Operand* p);
void printInterCode(InterCode* p);
int opCmp(Operand* a, Operand* b);

void setOpValue(Operand* op, char* name);

//用于优化代码
typedef struct LabelList_t{
	int labelNo;
	struct LabelList_t* nextLabel;
}LabelList;

void optInterCode();
void optGotoCode();
void deleteNullLebel();
void figureOutConstCalc();
void mergeAssignCode();
void deleteNullGoto();
#endif
