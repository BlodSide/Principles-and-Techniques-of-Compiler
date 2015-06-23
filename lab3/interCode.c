#include"interCode.h"

int temVarNo = 1;
int labelNo = 1;
int varNo = 1;

InterCode* codeHead = NULL;
InterCode* codeTail = NULL;

void insertCode(InterCode* p){
	assert(p!=NULL);
	if(codeHead==NULL){
		codeHead = p;
		codeTail = p;
	}
	else{
		p->preCode = codeTail;
		codeTail->nextCode = p;
		codeTail = p;
	}
}

void deleteCode(InterCode* p){
	assert(p!=NULL);
	if(p==codeHead){
		codeHead = p->nextCode;
	}
	else{
		p->preCode->nextCode = p->nextCode;
	}
	if(p==codeTail){
		codeTail = p->preCode;
	}
	else{
		p->nextCode->preCode = p->preCode;
	}
}

void fwriteOp(Operand* p, FILE* fp){
	assert(p!=NULL);
	char str[50];
	memset(str, 0, sizeof(str));
	switch(p->kind){
		case VAR_OP:
			assert(p->u.value!=NULL);
			sprintf(str, "%s ", p->u.value);
			fputs(str, fp);
			printf("%s",str);
			break;
		case CONST_OP:
			sprintf(str, "#%s ", p->u.value);
			fputs(str, fp);
			printf("%s",str);
			break;
		case ADDR_OP:
			fputs("*", fp);
			printf("*");
			assert(p->u.addr!=NULL);
			fwriteOp(p->u.addr, fp);
			break;
		case TEM_VAR_OP:
			sprintf(str, "t%d ", p->u.no);
			fputs(str, fp);
			printf("%s",str);
			break;
		case LABEL_OP:
			sprintf(str, "label%d ", p->u.no);
			fputs(str, fp);
			printf("%s",str);
			break;
		case FUNC_OP:
			sprintf(str, "%s ", p->u.value);
			fputs(str, fp);
			printf("%s",str);
			break;
	}
}

void fwriteAllCode(char* fileName){
	FILE* fp = fopen(fileName, "w");
	if(fp==NULL){
		printf("ERROR: open file \"%s\" fail!",fileName);
		return;
	}
	InterCode* p=codeHead;
	while(p!=NULL){
		switch(p->kind){
			case LABEL_CODE:
				fputs("LABEL ", fp);
				printf("LABEL ");
				fwriteOp(p->u.singleop.op, fp);
				fputs(": ", fp);
				printf(": ");
				break;
			case FUNCTION_CODE:
				fputs("FUNCTION ", fp);
				printf("FUNCTION ");
				fwriteOp(p->u.singleop.op, fp);
				fputs(": ", fp);
				printf(": ");
				break;
			case ASSIGN_CODE:
				fwriteOp(p->u.assign.left, fp);
				fputs(":= ", fp);
				printf(":= ");
				fwriteOp(p->u.assign.right, fp);
				break;
			case ADD_CODE:
				fwriteOp(p->u.doubleop.result, fp);
				fputs(":= ", fp);
				printf(":= ");
				fwriteOp(p->u.doubleop.op1, fp);
				fputs("+ ", fp);
				printf("+ ");
				fwriteOp(p->u.doubleop.op2, fp);
				break;
			case SUB_CODE:
				fwriteOp(p->u.doubleop.result, fp);
				fputs(":= ", fp);
				printf(":= ");
				fwriteOp(p->u.doubleop.op1, fp);
				fputs("- ", fp);
				printf("- ");
				fwriteOp(p->u.doubleop.op2, fp);
				break;
			case MUL_CODE:
				fwriteOp(p->u.doubleop.result, fp);
				fputs(":= ", fp);
				printf(":= ");
				fwriteOp(p->u.doubleop.op1, fp);
				fputs("* ", fp);
				printf("* ");
				fwriteOp(p->u.doubleop.op2, fp);
				break;
			case DIV_CODE:
				fwriteOp(p->u.doubleop.result, fp);
				fputs(":= ", fp);
				printf(":= ");
				fwriteOp(p->u.doubleop.op1, fp);
				fputs("/ ", fp);
				printf("/ ");
				fwriteOp(p->u.doubleop.op2, fp);
				break;
			case ADDRESS_CODE:
				fwriteOp(p->u.assign.left, fp);
				fputs(":= &", fp);
				printf(":= &");
				fwriteOp(p->u.assign.right, fp);
				break;
			case GOTO_CODE:
				fputs("GOTO ", fp);
				printf("GOTO ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			case IF_GOTO_CODE:
				fputs("IF ", fp);
				printf("IF ");
				fwriteOp(p->u.tribleop.x, fp);
				fputs(p->u.tribleop.relop, fp);
				printf("%s",p->u.tribleop.relop);
				fputs(" ", fp);
				printf(" ");
				fwriteOp(p->u.tribleop.y, fp);
				fputs("GOTO ", fp);
				printf("GOTO ");
				fwriteOp(p->u.tribleop.gotoLabel, fp);
				break;
			case RETURN_CODE:
				fputs("RETURN ", fp);
				printf("RETURN ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			case DEC_CODE:
				fputs("DEC ", fp);
				printf("DEC ");
				fwriteOp(p->u.dec.op, fp);
				char sizeStr[16];
				sprintf(sizeStr, "%d", p->u.dec.size);
				printf("%d",p->u.dec.size);
				fputs(sizeStr, fp);
				break;
			case ARG_CODE:
				fputs("ARG ", fp);
				printf("ARG ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			case CALL_CODE:
				fwriteOp(p->u.assign.left, fp);
				fputs(":= CALL ", fp);
				printf(":= CALL ");
				fwriteOp(p->u.assign.right, fp);
				break;
			case PARAM_CODE:
				fputs("PARAM ", fp);
				printf("PARAM ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			case READ_CODE:
				fputs("READ ", fp);
				printf("READ ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			case WRITE_CODE:
				fputs("WRITE ", fp);
				printf("WRITE ");
				fwriteOp(p->u.singleop.op, fp);
				break;
			default:
				assert(0);
		}
		fputs("\n", fp);
		printf("\n");
		p=p->nextCode;
	}
	fclose(fp);
}

void setOpValue(Operand* op, char* name){
	//这个函数可以修改 从而解决作用域问题
	op->u.value = name;
}

void optInterCode(){
	optGotoCode();
	deleteNullLebel();
	figureOutConstCalc();
	mergeAssignCode();
	deleteNullGoto();
	deleteNullLebel();
}

void optGotoCode(){
	//由于消除不必要的跳转
	InterCode* p =codeHead;
	while(p!=NULL){
		if(p->kind == IF_GOTO_CODE){
			//1.转化为fall模式的IF_GOTO: 将
			//IF v1 RELOP v2 GOTO LABEL 1
			//GOTO LABEL 2
			//LABEL 1
			//转化为
			//IF v1 ~RELOP v2 GOTO LABEL2
			//LABEL 1
			InterCode* c1 = p;
			InterCode* c2 = p->nextCode;
			if(c2==NULL){
				p = p->nextCode;
				continue;
			}
			InterCode* c3 = c2->nextCode;
			if(c3==NULL){
				p = p->nextCode;
				continue;
			}
			if(c2->kind==GOTO_CODE && c3->kind==LABEL_CODE && c1->u.tribleop.gotoLabel==c3->u.singleop.op && c2->u.singleop.op!=c3->u.singleop.op){
				c1->u.tribleop.gotoLabel = c2->u.singleop.op;
				deleteCode(c2);
				//下面将RELOP取反
				char* newRelop = malloc(3*sizeof(char));
				memset(newRelop, 0, 3*sizeof(char));
				if( strcmp(c1->u.tribleop.relop,"==")==0 ) strcpy(newRelop,"!=");
				if( strcmp(c1->u.tribleop.relop,"!=")==0 ) strcpy(newRelop,"==");
				if( strcmp(c1->u.tribleop.relop,"<")==0 ) strcpy(newRelop,">=");
				if( strcmp(c1->u.tribleop.relop,">")==0 ) strcpy(newRelop,"<=");
				if( strcmp(c1->u.tribleop.relop,"<=")==0 ) strcpy(newRelop,">");
				if( strcmp(c1->u.tribleop.relop,">=")==0 ) strcpy(newRelop,"<");
				c1->u.tribleop.relop = newRelop;
			}
		}
		else if(p->kind == GOTO_CODE){
			//2.去除冗余的GOTO：将
			//GOTO LABEL 1
			//LABEL 1
			//转化为
			//LABEL 1
			InterCode* c1 = p;
			InterCode* c2 = p->nextCode;
			if(c2==NULL){
				p = p->nextCode;
				continue;
			}
			if(c2->kind==LABEL_CODE && c1->u.singleop.op==c2->u.singleop.op){
				p = p->nextCode;
				deleteCode(c1);
				continue;
			}
		}
		p = p->nextCode;
	}
}

void deleteNullLebel(){
	//将没有GOTO语句或者IF_GOTO语句指向的LEBEL删去
	LabelList* usedLabelListHead = NULL;
	InterCode* p =codeHead;
	//建链表
	while(p!=NULL){
		if(p->kind==GOTO_CODE){
			LabelList* usedLabel = malloc(sizeof(LabelList));
			usedLabel->labelNo = p->u.singleop.op->u.no;
			usedLabel->nextLabel = usedLabelListHead;
			usedLabelListHead = usedLabel;
		}
		else if(p->kind==IF_GOTO_CODE){
			LabelList* usedLabel = malloc(sizeof(LabelList));
			usedLabel->labelNo = p->u.tribleop.gotoLabel->u.no;
			usedLabel->nextLabel = usedLabelListHead;
			usedLabelListHead = usedLabel;
		}
		p = p->nextCode;
	}
	p =codeHead;
	while(p!=NULL){
		if(p->kind==LABEL_CODE){
			int thisNo = p->u.singleop.op->u.no;
			LabelList* labelP = usedLabelListHead;
			while(labelP!=NULL){
				if(labelP->labelNo == thisNo){
					break;
				}
				labelP = labelP->nextLabel;
			}
			if(labelP==NULL){
				//说明链表中没有该no
				InterCode* tem = p;
				p = p->nextCode;
				deleteCode(tem);
				continue;
			}
		}
		p = p->nextCode;
	}
	//回收链表所占内存
	while(usedLabelListHead!=NULL){
		LabelList* tem = usedLabelListHead;
		usedLabelListHead = usedLabelListHead->nextLabel;
		free(tem);
	}
	//合并重复的Label
	p =codeHead;
	while(p!=NULL){
		InterCode* c1 = p;
		InterCode* c2 = p->nextCode;
		if(c2==NULL){
			p = p->nextCode;
			continue;
		}
		if(c1->kind==LABEL_CODE && c2->kind==LABEL_CODE){
			c1->u.singleop.op->u.no = c2->u.singleop.op->u.no;
			p = c2;
			deleteCode(c1);
			continue;
		}
		p = p->nextCode;
	}
}

void figureOutConstCalc(){
	//将常数之间的运算计算出来 
	//如果存储结果的是一个临时变量 则将该临时变量直接替换为该结果常数
	InterCode* p =codeHead;
	while(p!=NULL){
		if(p->kind==ADD_CODE || p->kind==SUB_CODE || p->kind==MUL_CODE || p->kind==DIV_CODE ){
			Operand* result = p->u.doubleop.result;
			Operand* op1 = p->u.doubleop.op1;
			Operand* op2 = p->u.doubleop.op2;
			if(op1->kind==CONST_OP && op2->kind==CONST_OP){
				int op1Int=atoi(op1->u.value);
				int op2Int=atoi(op2->u.value);
				int resultInt;
				switch(p->kind){
					case ADD_CODE:
						resultInt = op1Int+op2Int;
						break;
					case SUB_CODE:
						resultInt = op1Int-op2Int;
						break;
					case MUL_CODE:
						resultInt = op1Int*op2Int;
						break;
					case DIV_CODE:
						resultInt = op1Int/op2Int;
							break;
					default:assert(0);
				}
				char* resultStr = malloc(16*sizeof(char));
				sprintf(resultStr, "%d", resultInt);
				if(result->kind==TEM_VAR_OP){
					result->kind = CONST_OP;
					result->u.value = resultStr;
					InterCode* tem = p;
					p = p->nextCode;
					deleteCode(tem);
					continue;
				}
				else{
					p->kind = ASSIGN_CODE;
					p->u.assign.left = result;
					p->u.assign.right = op1;
					op1->u.value = resultStr;
				}
			}
		}
		p = p->nextCode;
	}
}

void mergeAssignCode(){
	//观察赋值语句前后语句 在可以直接替换的情况下直接进行变量替换 从而将赋值语句合并
	InterCode* p =codeHead;
	while(p!=NULL){
		InterCode* c1 = p;
		InterCode* c2 = p->nextCode;
		if(c2!=NULL && c1->kind==ASSIGN_CODE && c2->kind!=DEC_CODE ){
			//当前语句下一句的右值等于当前语句的左值 并且该左值是临时的 则该左值是不需要的
			if(c2->u.doubleop.op1==c1->u.assign.left && c1->u.assign.left->kind==TEM_VAR_OP){
				c2->u.doubleop.op1 = c1->u.assign.right;
				p = p->nextCode;
				deleteCode(c1);
				continue;
			}
			else if(c2->u.doubleop.op2==c1->u.assign.left){
				c2->u.doubleop.op2 = c1->u.assign.right;
				p = p->nextCode;
				deleteCode(c1);
				continue;
			}
		}
		InterCode* c0 = p->preCode;
		if(c0!=NULL && c1->kind==ASSIGN_CODE && c0->kind!=DEC_CODE && c0->kind!=IF_GOTO_CODE ){
			//当前语句上一句的左值等于当前语句的右值 并且该右值是临时的 则该右值是不需要的
			if(c0->u.doubleop.result==c1->u.assign.right && c1->u.assign.right->kind==TEM_VAR_OP){
				c0->u.doubleop.result=c1->u.assign.left;
				p = p->nextCode;
				deleteCode(c1);
				continue;
			}
		}
		p = p->nextCode;
	}
}

void deleteNullGoto(){
	//RETURN语句后面的GOTO永远不会运行到
	InterCode* p =codeHead;
	while(p!=NULL){
		if(p->kind == RETURN_CODE){
			InterCode* c1 = p;
			InterCode* c2 = p->nextCode;
			if(c2==NULL){
				p = p->nextCode;
				continue;
			}
			if(c2->kind==GOTO_CODE)
				deleteCode(c2);
		}
		p = p->nextCode;
	}
}
