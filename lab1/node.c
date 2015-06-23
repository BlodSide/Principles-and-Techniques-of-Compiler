#include "node.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
extern int yylineno;

Node* initNode(char* name, char* value){
//生成一个新节点
	Node* p = (Node*)malloc(sizeof(Node));
	p->lineno = yylineno;
	strcpy(p->name, name);
	strcpy(p->value, value);
	p->bro = NULL;
	p->child = NULL;
	return p;
}
void addChild(Node* parent,Node* child){
//将child设为parent的子节点
//注意：是当前存在的子节点的前面插入一个新的子节点（倒序插入）
	if(parent != NULL && child != NULL){
		child->bro = parent->child;
		parent->child = child;
		parent->lineno = child->lineno;
	}
}
void printTree(Node* root, int n){
//将树按照实验要求打印
//调用的时候 root使用根结点 n使用0
	if(root == NULL)
		return;
	int i;
	for(i=0; i<n; i++)
		printf("  ");
	if(root->child == NULL){
		if(strcmp(root->name,"FLOAT") == 0)
			printf("INT: %f\n", atof(root->value));
		else if(strcmp(root->name,"INT") == 0)
			printf("INT: %ld\n", strtol(root->value, NULL, 0));
		else if(strcmp(root->name,"ID") == 0|strcmp(root->name,"TYPE") == 0)
			printf("%s: %s\n", root->name, root->value);
		else
			printf("%s\n", root->name);
	}
	else{
		printf("%s (%d)\n", root->name, root->lineno);
		Node* p = root->child;
		while(p != NULL){
			printTree(p, n+1);
			p = p->bro;
		}
	}
}
