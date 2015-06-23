#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <string.h>
typedef struct Node_t{
	char name[16];
	char value[32];
	int lineno;
	struct Node_t* child;
	struct Node_t* bro;
}Node;
Node* initNode(char* name, char* value);
void addChild(Node* parent, Node* child);
void printTree(Node* root, int n);

#endif
