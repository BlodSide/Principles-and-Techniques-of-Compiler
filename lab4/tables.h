#ifndef _TABLES_H_
#define _TABLES_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define TRUE 1
#define FALSE 0

#define INT_SIZE 4
#define FLOAT_SIZE 4

#define INT_TYPE 11
#define FLOAT_TYPE 12
#define BASIC_KIND 1
#define ARRAY_KIND 2
#define STRUCT_KIND 3

#define REDEFINE_ERROR -2
#define INCONSISTENT_DELARATION_ERROR -3
// 0x7FFF == 32767
#define HASH_TABLE_SIZE 32768

#define STACK_SIZE 200

typedef struct SymDes_t{
	char* name;
	//hash������ͬhashֵ��λ���ϵ������nextָ���preָ��
	struct SymDes_t* nextInHash;
	struct SymDes_t* preInHash;
	//���ſ�����ĳ����ṹ��
	struct SymDes_t* nextInList;
	//BASIC_KIND or ARRAY_KIND or STRUCT_KIND
	int kind;
	union{
		//INT_TYPE or FLOAT_TYPE
		int basic;
		struct{
			//�����Ԫ������
			struct SymDes_t* formBy;
			//�����Ԫ�ظ���
			int size;
		} array;
		//�ṹ������
		struct StrucDes_t* struc;
	} detail;
	//������������ �����0�Ļ���Ϊȫ��
	int depth;
	//ջ�������ָ��
	struct SymDes_t* nextInStack;
} SymDes;

typedef struct StrucDes_t{
	char* name;
	struct StrucDes_t* nextInHash;
	struct StrucDes_t* preInHash;
	SymDes* fieldList;
	int depth;
	struct StrucDes_t* nextInStack;
} StrucDes;

typedef struct FuncDes_t{
	char* name;
	//hash������ͬhashֵ��λ���ϵ������nextָ��
	struct FuncDes_t* nextInHash;
	int isDef;
	SymDes* retType;
	SymDes* paraList;
	int lineno;
} FuncDes;

unsigned int strHash(char *str);
/*
 * �������ݽṹ�ĺ���
 */
int symDesCmp(SymDes* a, SymDes* b);
int symDesListCmp(SymDes* a, SymDes* b);
int typeCmp(SymDes* a, SymDes* b); //���Ƚ�name�ֶ�
int typeListCmp(SymDes* a, SymDes* b); //���Ƚ�name�ֶ�
int sizeofType(SymDes* type);
/*
 * ���ڱ�ĺ���
 */
void initTable();
int insertSymTable(SymDes* p);
void insertParaOfFunc(FuncDes* p);
int insertStrucTable(StrucDes* p);
SymDes* findSymDes(char *name);
FuncDes* findFuncDes(char* name);
StrucDes* findStrucDes(char* name);
void checkFuncDef();
/*
 * ����debug�ĺ���
 */
void printType( SymDes* p );
void printTypeList( SymDes* p );

void tablesPush();
void tablesPop();

#endif
