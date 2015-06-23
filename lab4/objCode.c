#include "objCode.h"

RegDes regList[REGNUM];
VarDes* varList = NULL;
int offset = FSIZE;

void clearVarList(){
	while(varList!=NULL){
		VarDes* tem = varList;
		varList = varList->next;
		free(tem);
	}
}

void addVarToList(VarDes* p){
	assert(p!=NULL);
	p->next = varList;
	varList = p;
}

void stVar(VarDes* var, FILE* fp){
	char stCode[64];
	if(var->op->kind == ADDR_OP){
		int addrReg = allocateRegForOp(var->op->u.addr, fp);
		memset(stCode, 0, sizeof(stCode));
		sprintf(stCode, "sw $%s, 0($%s)\n", regList[var->regNo].name, regList[addrReg].name);
	}
	else if(var->op->kind == VAR_OP || var->op->kind == TEM_VAR_OP){
		memset(stCode, 0, sizeof(stCode));
		sprintf(stCode, "subu $v1, $fp, %d\n", var->offset);
		fputs(stCode, fp);
		memset(stCode, 0, sizeof(stCode));
		sprintf(stCode, "sw $%s, 0($v1)\n", regList[var->regNo].name);
	}
	else
		assert(0);
	regList[var->regNo].old = 0;
	regList[var->regNo].var = NULL;
	var->regNo = -1;
	fputs(stCode, fp);
}

void ldVar(VarDes* var, FILE* fp){
	//需要将内存中的变量load到寄存器中
	char ldCode[64];
	if(var->op->kind == VAR_OP || var->op->kind == TEM_VAR_OP){
		memset(ldCode, 0, sizeof(ldCode));
		sprintf(ldCode, "subu $v1, $fp, %d\n", var->offset);
		fputs(ldCode, fp);
		memset(ldCode, 0, sizeof(ldCode));
		sprintf(ldCode, "lw $%s, 0($v1)\n", regList[var->regNo].name);
	}
	else if(var->op->kind == CONST_OP){
		memset(ldCode, 0, sizeof(ldCode));
		sprintf(ldCode, "li $%s, %s\n", regList[var->regNo].name, var->op->u.value);
	}
	else if(var->op->kind == ADDR_OP){
		int addrReg = allocateRegForOp(var->op->u.addr, fp);
		memset(ldCode, 0, sizeof(ldCode));
		sprintf(ldCode, "lw $%s, 0($%s)\n", regList[var->regNo].name, regList[addrReg].name);
	}
	else{
		//不会出现 LABEL_OP FUNC_OP 
		assert(0);
	}
	fputs(ldCode, fp);
}


void initAllRegDes(){
	int i;
	for(i=0; i<REGNUM; i++){
		regList[i].old = 0;
		regList[i].var = NULL;
	}
	//为了方便编码 寄存器描述符表的序号并不是寄存器的实际序号
	strcpy(regList[0].name, "t0");
	strcpy(regList[1].name, "t1");
	strcpy(regList[2].name, "t2");
	strcpy(regList[3].name, "t3");
	strcpy(regList[4].name, "t4");
	strcpy(regList[5].name, "t5");
	strcpy(regList[6].name, "t6");
	strcpy(regList[7].name, "t7");
	strcpy(regList[8].name, "t8");
	strcpy(regList[9].name, "t9");
	strcpy(regList[10].name, "s0");
	strcpy(regList[11].name, "s1");
	strcpy(regList[12].name, "s2");
	strcpy(regList[13].name, "s3");
	strcpy(regList[14].name, "s4");
	strcpy(regList[15].name, "s5");
	strcpy(regList[16].name, "s6");
	strcpy(regList[17].name, "s7");
	/*
	strcpy(regList[18].name, "a0");
	strcpy(regList[19].name, "a1");
	strcpy(regList[20].name, "a2");
	strcpy(regList[21].name, "a3");
	*/
}

void rstAllReg(){
	//只重置变量寄存器
	int i;
	for(i=0; i<REGNUM; i++){
		regList[i].old = 0;
		regList[i].var = NULL;
	}
}

void freeOneReg(int regNo){
	//释放regNo对应的寄存器
	VarDes* tem = varList;
	while(tem!=NULL){
		if(tem->regNo == regNo)
			tem->regNo = -1;
		tem = tem->next;
	}
	regList[regNo].old = 0;
	regList[regNo].var = NULL;
}

int allocateRegForOp(Operand* op, FILE* fp){
	//在varList中寻找op对应的变量的VarDes
	VarDes* var = varList;
	while(var!=NULL){
		if(opCmp(var->op, op)==0)
			break;
		var = var->next;
	}
	
	//如果op对应的变量第一次被使用 则记录栈中相对于栈底($fp)的位置(偏移量)
	int isFirstUsed = FALSE;
	if(var == NULL){
		isFirstUsed = TRUE;
		var = malloc(sizeof(VarDes));
		memset(var, 0, sizeof(VarDes));
		var->op = op;
		var->regNo = -1;
		if(var->op->kind == VAR_OP || var->op->kind == TEM_VAR_OP){
			offset += 4;
			var->offset = offset;
		}
		if(var->op->kind != CONST_OP)
			addVarToList(var);
	}
	
	//如果op对应的变量没有被分配寄存器 则为其分配寄存器
	if(var->regNo < 0){
		int regNo = getReg(fp);
		var->regNo = regNo;
		regList[regNo].var = var;
		
		if(var->op->kind == CONST_OP||var->op->kind == ADDR_OP||(isFirstUsed==FALSE && var->offset>=0)){
			//不是第一次出现的变量 
			ldVar(var, fp);
		}
	}
	
	//每次寄存器中的变量被使用 old字段就会置零
	regList[var->regNo].old = 0;
	return var->regNo;
}

int getReg(FILE* fp){
	//获取一个可用的寄存器
	//所有没被释放的变量寄存器的old都加一
	int i;
	int freedReg = -1;
	for(i=0; i<REGNUM; i++){
		if(regList[i].var!=NULL)
			regList[i].old++;
		else
			freedReg = i;
	}
	//如果有没被释放的变量寄存器 则直接使用它
	if(freedReg>0)
		return freedReg;
	
	//找出最久未被使用的reg
	int oldestReg = -1;
	int oldest = -1;
	for(i=0; i<REGNUM; i++){
		if(regList[i].old>oldest){
			oldest = regList[i].old;
			oldestReg = i;
		}
	}
	
	VarDes* var = varList;
	while(var!=NULL){
		if(var!=regList[oldestReg].var)
			var = var->next;
		else
			break;
	}
	if(var!=NULL){
		//将oldestReg中的变量store
		assert(oldestReg==var->regNo);
		stVar(var, fp);
	}
	else
		assert(0);
	return oldestReg;
}

void fwriteAllObjCode(char* fileName){
	initAllRegDes();
	FILE* fp = fopen(fileName, "w");
	if(fp==NULL){
		printf("ERROR: open file \"%s\" fail!",fileName);
		return;
	}
	fputs(".data\n", fp);
	fputs("_prompt: .asciiz \"Enter an integer:\"\n", fp);
	fputs("_ret: .asciiz \"\\n\"\n", fp);
	fputs(".globl main\n", fp);
	fputs(".text\n", fp);
	fputs("\n", fp);
	fputs("read:\n", fp);
	fputs("subu $sp, $sp, 8\n", fp); 
	fputs("sw $ra, 4($sp)\n", fp);
	fputs("sw $fp, 0($sp)\n", fp);
	fputs("addi $fp, $sp, 8\n", fp);
	fputs("li $v0, 4\n", fp);
	fputs("la $a0, _prompt\n", fp);
	fputs("syscall\n", fp);
	fputs("li $v0, 5\n", fp);
	fputs("syscall\n", fp);
	fputs("subu $sp, $fp, 8\n", fp);
	fputs("lw $ra, 4($sp)\n", fp);
	fputs("lw $fp, 0($sp)\n", fp);
	fputs("jr $ra\n", fp);
	fputs("\n", fp);
	fputs("write:\n", fp);
	fputs("subu $sp, $sp, 8\n", fp); 
	fputs("sw $ra, 4($sp)\n", fp);
	fputs("sw $fp, 0($sp)\n", fp);
	fputs("addi $fp, $sp, 8\n", fp);
	fputs("li $v0, 1\n", fp);
	fputs("syscall\n", fp);
	fputs("li $v0, 4\n", fp);
	fputs("la $a0, _ret\n", fp);
	fputs("syscall\n", fp);
	fputs("subu $sp, $fp, 8\n", fp);
	fputs("lw $ra, 4($sp)\n", fp);
	fputs("lw $fp, 0($sp)\n", fp);
	fputs("move $v0, $0\n", fp);
	fputs("jr $ra\n", fp);
	
	InterCode* ir = codeHead;
	while(ir!=NULL){
		fwriteOneObjCode(ir, fp);
		ir = ir->nextCode;
	}
	fclose(fp);
}

int argc = 0;

void fwriteOneObjCode(InterCode* ir, FILE* fp){
	printInterCode(ir);
	switch(ir->kind){
		case LABEL_CODE:{
			//一个语句块的边界 应该将所有变量store
			VarDes* var = varList;
			while(var!=NULL){
				if(var->regNo>=0)
					stVar(var, fp);
				var = var->next;
			}
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "label%d:\n", ir->u.singleop.op->u.no);
			fputs(str, fp);
			break;
		}
		case FUNCTION_CODE:{
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "\n%s:\n", ir->u.singleop.op->u.value);
			fputs(str, fp);
			
			//Prologue
			//将ra寄存器和fp寄存器压栈
			memset(str, 0, sizeof(str));
			sprintf(str, "subu $sp, $sp, %d\n", FSIZE);
			fputs(str, fp);
			fputs("sw $ra, 4($sp)\n", fp);
			fputs("sw $fp, 0($sp)\n", fp);
			//重置fp寄存器
			fputs("addi $fp, $sp, 8\n", fp);
			
			//清空varList 重置所有RegDes和offset
			clearVarList();
			rstAllReg();
			offset = FSIZE;
			
			ir = ir->nextCode;
			int i = 0;
			while(ir->kind == PARAM_CODE ){
				int paramReg = allocateRegForOp(ir->u.singleop.op, fp);
				memset(str, 0, sizeof(str));
				if(i<4){
					//前四个参数暂存在a0~a3寄存器中
					sprintf(str, "move $%s, $a%d\n", regList[paramReg].name, i);
				}
				else{
					//余下的参数在栈中
					sprintf(str, "lw $%s, %d($sp)\n", regList[paramReg].name, FSIZE+(i-4)*4);
				}
				fputs(str, fp);
				i++;
				ir = ir->nextCode;
			}
			break;
		}
		case ASSIGN_CODE:{
			Operand* leftOp = ir->u.assign.left;
			Operand* rightOp = ir->u.assign.right;
			int leftReg = allocateRegForOp(leftOp, fp);
			int rightReg = allocateRegForOp(rightOp, fp);
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "move $%s, $%s\n", regList[leftReg].name, regList[rightReg].name);
			fputs(str, fp);
			if(leftOp->kind == ADDR_OP)
				stVar(regList[leftReg].var, fp);
			break;
		}
		case ADD_CODE:
		case SUB_CODE:
		case MUL_CODE:
		case DIV_CODE:{
			char op[4];
			memset(op, 0, sizeof(op));
			switch(ir->kind){
				case ADD_CODE:
					strcpy(op, "add");
					break;
				case SUB_CODE:
					strcpy(op, "sub");
					break;
				case MUL_CODE:
					strcpy(op, "mul");
					break;
				case DIV_CODE:
					strcpy(op, "div");
					break;
			}
			int op1Reg = allocateRegForOp(ir->u.doubleop.op1, fp);
			int op2Reg = allocateRegForOp(ir->u.doubleop.op2, fp);
			int resultReg = allocateRegForOp(ir->u.doubleop.result, fp);
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "%s $%s, $%s, $%s\n", op, regList[resultReg].name, regList[op1Reg].name, regList[op2Reg].name);
			fputs(str, fp);
			if(ir->u.doubleop.result->kind == ADDR_OP)
				stVar(regList[resultReg].var, fp);
			break;
		}
		case ADDRESS_CODE:{
			//已经在DEC_CODE部分处理过
			break;
		}
		case GOTO_CODE:{
			//一个语句块的边界 应该将所有变量store
			VarDes* var = varList;
			while(var!=NULL){
				if(var->regNo>=0)
					stVar(var, fp);
				var = var->next;
			}
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "j label%d\n", ir->u.singleop.op->u.no);
			fputs(str, fp);
			break;
		}
		case IF_GOTO_CODE:{
			//一个语句块的边界 应该将所有变量store
			VarDes* var = varList;
			while(var!=NULL){
				if(var->regNo>=0)
					stVar(var, fp);
				var = var->next;
			}
			char op[4];
			if(strcmp(ir->u.tribleop.relop,"==")==0)
				strcpy(op,"beq");
			else if(strcmp(ir->u.tribleop.relop,"!=")==0)
				strcpy(op,"bne");
			else if(strcmp(ir->u.tribleop.relop,">")==0)
				strcpy(op,"bgt");
			else if(strcmp(ir->u.tribleop.relop,"<")==0)
				strcpy(op,"blt");
			else if(strcmp(ir->u.tribleop.relop,">=")==0)
				strcpy(op,"bge");
			else if(strcmp(ir->u.tribleop.relop,"<=")==0)
				strcpy(op,"ble");
			else
				assert(0);
			int xReg = allocateRegForOp(ir->u.tribleop.x, fp);
			int yReg = allocateRegForOp(ir->u.tribleop.y, fp);
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "%s $%s, $%s, label%d\n", op, regList[xReg].name, regList[yReg].name, ir->u.tribleop.gotoLabel->u.no);
			fputs(str, fp);
			break;
		}
		case RETURN_CODE:{
			Operand* retOp = ir->u.singleop.op;
			int retReg;
			retReg = allocateRegForOp(retOp, fp);
			//Epilogue
			//sp寄存器指向栈帧
			fputs("subu $sp, $fp, 8\n", fp);
			//恢复ra寄存器和fp寄存器的值
			fputs("lw $ra, 4($sp)\n", fp);
			fputs("lw $fp, 0($sp)\n", fp);
			//将返回值放入v0寄存器内 
			//由于函数即将返回 v0中的变量无需store
			char str[64];
			memset(str, 0, sizeof(str));
			if(retOp->kind==VAR_OP || retOp->kind==TEM_VAR_OP)
				sprintf(str, "move $v0, $%s\n", regList[retReg].name);
			else if(retOp->kind==CONST_OP)
				sprintf(str, "li $v0, %s\n", retOp->u.value);
			fputs(str, fp);
			fputs("jr $ra\n", fp);
			break;
		}
		case DEC_CODE:{
			// DEC_CODE 的下一条中间代码一定是 ADDRESS_CODE
			// 且 DEC_CODE 的op一定是 ADDRESS_CODE 的right
			// 根据这一特性可以进行优化
			// 从而节省指令
			assert(ir->nextCode->kind == ADDRESS_CODE);
			assert(ir->u.dec.op == ir->nextCode->u.assign.right);
			int regNo = allocateRegForOp(ir->nextCode->u.assign.left, fp);
			
			//开空间
			offset += ir->u.dec.size;
			//将空间的首地址存入寄存器
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "subu $%s, $fp, %d\n", regList[regNo].name, offset);
			fputs(str, fp);
			
			break;
		}
		case ARG_CODE:{
			argc++;
			break;
		}
		case CALL_CODE:{
			//CALL是一个语句块的边界 应该将所有变量store
			VarDes* var = varList;
			while(var!=NULL){
				if(var->regNo>=0)
					stVar(var, fp);
				var = var->next;
			}
			InterCode* irPre = ir;
			//修改sp寄存器 为需要压栈的参数申请空间
			char str[64];
			if(argc>4){
				memset(str, 0, sizeof(str));
				sprintf(str, "subu $sp, $fp, %d\n", offset+(argc-4)*4);
				fputs(str, fp);
			}
			else{
				memset(str, 0, sizeof(str));
				sprintf(str, "subu $sp, $fp, %d\n", offset);
				fputs(str, fp);
			}
			//参数压栈
			int i;
			for(i=0; i<argc; i++){
				irPre = irPre->preCode;
				int argReg = allocateRegForOp(irPre->u.singleop.op, fp);
				memset(str, 0, sizeof(str));
				if(i<4){
					sprintf(str, "move $a%d, $%s\n", i, regList[argReg].name);
				}
				else{
					sprintf(str, "sw $%s, %d($sp)\n", regList[argReg].name, (i-4)*4);
				}
				fputs(str, fp);
			}
			//跳转
			memset(str, 0, sizeof(str));
			sprintf(str, "jal %s\n", ir->u.assign.right->u.value);
			fputs(str, fp);
			argc = 0;
			//取v0寄存器中的返回值
			int retReg = allocateRegForOp(ir->u.assign.left, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "move $%s, $v0\n", regList[retReg].name);
			fputs(str, fp);
			break;
		}
		case PARAM_CODE:{
			//已经在FUNCTION_CODE部分处理过
			break;
		}
		case READ_CODE:{
			//设置栈顶指针到正确位置
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "subu $sp, $fp, %d\n", offset);
			fputs(str, fp);
			//跳转
			fputs("jal read\n", fp);
			//取返回值
			int retReg = allocateRegForOp(ir->u.singleop.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "move $%s, $v0\n", regList[retReg].name);
			fputs(str, fp);
			break;
		}
		case WRITE_CODE:{
			//设置栈顶指针到正确位置
			char str[64];
			memset(str, 0, sizeof(str));
			sprintf(str, "subu $sp, $fp, %d\n", offset);
			fputs(str, fp);
			//传参数到a0寄存器
			int paramReg = allocateRegForOp(ir->u.singleop.op, fp);
			memset(str, 0, sizeof(str));
			sprintf(str, "move $a0, $%s\n", regList[paramReg].name);
			fputs(str, fp);
			//跳转
			fputs("jal write\n", fp);
			break;
		}
		default:
			assert(0);
	}
}
