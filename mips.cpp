#include "mips.h"
#include "error.h"
#include<algorithm>
int imListCnt = 0;
/*
totMap<labelName,curMap<VarName,VarValue>>
*/
set<string> labNameSet;//store labels
map<string,string> dataMap;//store strings
map<string, string> curConstMap;
map<string, map<string, string>> totConstMap;
map<string, map<string, int>> totMap;
map<string, int> curMap;
map<string, map<string, int>> totParaMap;
map<string, int> curParaMap;

string sizeOfFunc = "#size";
string eof = "enD____OFLIne";
string prefix = "__K__";
string globalstr = "globeel";
string globalvarLabel = prefix + globalstr + "var";
string curConstMapName = prefix + globalstr;
string curLabel = prefix + globalstr;
int labnamecnt = 0;
//int glboffset = 0;
int offset = 0;
int paraOffset = 0;
// reg alloc
#define MAX_T_REGS 9
#define DIRTY 1
#define USING 0
#define USABLE -1
map<string, string> regToId;
map<string, string> idToReg;
map<string, int> regStatus;

void initReg() {
	for (int i = 0; i <= MAX_T_REGS; i++) {
		string t = "$t" + to_string(i);
		regStatus[t] = USABLE;
	}
	idToReg.clear();
	regToId.clear();
}

bool varWillBeUsed(int line,string iden,int back_range) {
	if (iden[0] != '@')//only neglect temp vars
		return true;
	for (int j = line + 1; j < imList.size(); j++) {
		imTok temp = imList[j];
		if (j != line && (temp.a == iden || temp.b == iden || temp.c == iden)) {
			return true;
		}
	}
	return false;
}

void saveReg(string currentlab,int line,int back_range) {
	for (int i = 0; i <= MAX_T_REGS; i++) {
		string t = "$t" + to_string(i);
		if (regToId.find(t) == regToId.end())//unalloc
			continue;
		string oldv = regToId[t];
		if (regStatus[t] >= DIRTY && varWillBeUsed(line,oldv, back_range)/*todo : not used again*/) {
			int s = checkIdenfr(oldv, currentlab);
			if (s != -1) {
				mprint("sw " + t + ",-" + to_string(s * 4) + "($sp)");
			}
			else {
				s = checkIdenfr(oldv, prefix + globalstr);
				mprint("sw " + t + "," + globalvarLabel + "+" + to_string(s * 4));
			}
		}
	}
	initReg();
}

string isRegPara(string id, string currentlab) {
	if (totParaMap[currentlab].find(id) != totParaMap[currentlab].end()) {
		int num = totParaMap[currentlab][id];
		if (num <= 3)
			return "$a" + to_string(num);
	}
	return "";
}

string getReg(string id,string currentlab,int write,int line) {// allocate a register to id

	if (idToReg.find(id) != idToReg.end()) {
		return idToReg[id];
	}
	for (int i = 0; i <= MAX_T_REGS; i++) {// still have usable regs
		string t = "$t" + to_string(i);
		if (regStatus[t] == USABLE) {
			regStatus[t] = USING + write;
			regToId[t] = id;
			idToReg[id] = t;
			return t;
		}
	}
	// all regs have been allocated,then
	int maxDis = 0;
	string maxId;
	string maxReg;
	for (int i = 0; i <= MAX_T_REGS; i++) {//opt replacing
		string t = "$t" + to_string(i);
		string iden = regToId[t];
		int dis = imList.size() - line - 1;
		for (int j = line + 1; j < imList.size(); j++) {
			imTok temp = imList[j];
			if (temp.a == iden || temp.b == iden || temp.c == iden) {
				dis = j - line - 1;
				break;
			}
		}
		maxDis = max(maxDis, dis);
		if (maxDis == dis) {
			maxId = iden;
			maxReg = t;
		}
	}

	string s = "#now change reg " + maxReg + " from id:" + maxId + " to id : " + id;
	printf("%s\n",s.c_str());

	//save old reg
	string oldv = regToId[maxReg];
	if (regStatus[maxReg] >= DIRTY && varWillBeUsed(line,oldv,0)) {
		int s = checkIdenfr(oldv, currentlab);
		if (s != -1) {
			mprint("sw " + maxReg + ",-" + to_string(s * 4) + "($sp)");
		}
		else {
			s = checkIdenfr(oldv, prefix + globalstr);
			mprint("sw " + maxReg + "," + globalvarLabel + "+" + to_string(s * 4));
		}
	}
	idToReg.erase(oldv);

	//alloc new reg
	regToId[maxReg] = id;
	idToReg[id] = maxReg;
	regStatus[maxReg] = USING + write;
	return maxReg;
}

string loadReg(string id, string currentlab,int write,int line) {//allocate a reg, and set it to the contents of idenfier "id"(if write = 0) , then return its name
	string tempP = isRegPara(id, currentlab);
	if (tempP != "")
		return tempP;

	if (!isIdenfr(id))
		return id;//not a valid id

	string reg;//get a register
	if (idToReg.find(id) != idToReg.end()) {//already allocated,if write,then change regStatus
		reg = idToReg[id];
		regStatus[reg] = regStatus[reg] + write;
		return reg;
	}
	else {//alloc a reg
		reg = getReg(id, currentlab, write, line);
	}

	//set id's contents to the register
	if (id == "RET") {
		//mprint("move " + reg + "," + "$v0");
		return "$v0";
	}

	else if (write == 0){//need id's value , while id not in active regs, then load value from stack
		int s = checkIdenfr(id, currentlab);
		if (s != -1) {
			mprint("lw " + reg + ",-" + to_string(s * 4) + "($sp)");
		}
		else {
			s = checkIdenfr(id, prefix + globalstr);
			mprint("lw " + reg + "," + globalvarLabel + "+" + to_string(s * 4));
		}
	}
	return reg;
}

void setReg(string id, string reg, string currentlab) {//set "reg" to the contents of idenfier "id" ,only works in $a0..a3,$v0
	if (idToReg.find(id) != idToReg.end()) {//already in register,then move 
		mprint("move " + reg + "," + idToReg[id]);
		return ;
	}
	else if (isIdenfr(id)) {//not allocated
		int s = checkIdenfr(id, currentlab);
		if (s != -1) {
			mprint("lw " + reg + ",-" + to_string(s * 4) + "($sp)");
		}
		else {
			s = checkIdenfr(id, prefix + globalstr);
			mprint("lw " + reg + "," + globalvarLabel + "+" + to_string(s * 4));
		}
	}
	else {// is number
		mprint("li " + reg + "," + id);
	}
}

string getLabName(string name) { //generate and return a labelname of string.
	if (dataMap.find(name) != dataMap.end())
		return dataMap[name];
	string x = prefix + to_string(labnamecnt++);// label name
	if (labNameSet.find(x) != labNameSet.end())
		x += labnamecnt++;
	labNameSet.insert(x);
	dataMap[name] = x;
	return x;
}

void mprint(string s) {
	printf("%s\n", s.c_str());
}

void mprintc(midType type,string a,string b,string c) {
	if (COMMENT_ON)
		printf("#%s,%s,%s,%s\n", imoutput[type].c_str(), a.c_str(), b.c_str(), c.c_str());
}

void escapedChar() {
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		string t;
		if (temp.type == PRINTF && temp.a != "") {
			for (int j = 0; j < temp.a.size(); j++) {
				if (temp.a[j] == '\\')
					t += "\\\\";
				else
					t += temp.a[j];
			}
			temp.a = t;
			imList[i] = temp;
		}
		
	}
}

void stackAlloc() {

}

void getConstVal(string label, string id,string &in) {//check and return const "id" in function range "label"
	string ret = "";
	if (label != "" && totConstMap[label].find(id) != totConstMap[label].end())
		ret = totConstMap[label][id];
	else if (totConstMap[prefix + globalstr].find(id) != totConstMap[prefix + globalstr].end())
		ret = totConstMap[prefix + globalstr][id];
	if (ret != "")
		in = ret;
}

void constReplace() {
	for (int i = 0; i < imList.size(); i++) {//set const symtable(totConstMap)
		imTok temp = imList[i];
		if (temp.type == CONSTDEF) {
			string con = temp.b;
			string val = temp.c;
			if (temp.a == "int") {
				curConstMap[con] = val;
			}
			else if (temp.a == "char") {
				int x = (int)(val[0]);
				curConstMap[con] = to_string(x);
			}
		}
		else if (temp.type == LABEL && temp.b == "func") {
			totConstMap[curConstMapName] = curConstMap;
			curConstMap.clear();
			curConstMapName = temp.a;
		}
	}
	totConstMap[curConstMapName] = curConstMap;

	string currentlab = "";
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == LABEL && temp.b == "func") {
			currentlab = temp.a;
		}
		else if (temp.type == PRINTF) {
			getConstVal(currentlab, temp.b, temp.b);
			imList[i].b = temp.b;
		}
		else if (temp.type >= BLE && temp.type <= BEQ) {
			getConstVal(currentlab, temp.b, temp.b);
			imList[i].b = temp.b;
			getConstVal(currentlab, temp.a, temp.a);
			imList[i].a = temp.a;
		}
		else if (temp.type == LOAD) {
			getConstVal(currentlab, temp.b, temp.b);
			imList[i].b = temp.b;
		}
		else if (temp.type == STORE) {
			getConstVal(currentlab, temp.b, temp.b);
			imList[i].b = temp.b;
			getConstVal(currentlab, temp.a, temp.a);
			imList[i].a = temp.a;
		}
		else if (temp.type == ADD || temp.type == SUB || temp.type == MULTI || temp.type == DIVIDE) {
			getConstVal(currentlab, temp.b, temp.b);
			getConstVal(currentlab, temp.c, temp.c);
			imList[i].b = temp.b;
			imList[i].c = temp.c;
		}
	}
}

void insertSymTable(string a) {//insert symbol to current table
	if (isIdenfr(a)) {//a must be idenfiers
		if (curMap.find(a) == curMap.end() && curParaMap.find(a) == curParaMap.end())
			if (totMap[prefix + globalstr].find(a) == totMap[prefix + globalstr].end())
				curMap[a] = offset++;
	}
}

void setSymTable() {//set var symtable(totMap\totParaMap)
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == VARDEF) {
			curMap[temp.b] = offset++;
		}
		else if (temp.type == PARA) {
			curMap[temp.b] = offset++;
			curParaMap[temp.b] = paraOffset++;
		}
		else if (temp.type == LABEL && temp.b == "func") {
			curMap[sizeOfFunc] = offset;
			if (curLabel != "") {
				totMap[curLabel] = curMap;
				totParaMap[curLabel] = curParaMap;
			}
			curParaMap.clear();
			curMap.clear();
			curLabel = temp.a;
			offset = 0;
			paraOffset = 0;
		}
		else if (temp.type == ARRAYDEF) {
			if (isIdenfr(temp.a)) {//a must be idenfiers
				if (curMap.find(temp.a) == curMap.end())
					curMap[temp.a] = offset;
					offset += atoi(temp.b.c_str());
			}
		}
		else if (temp.type == ADD || temp.type == SUB || temp.type == MULTI || temp.type == DIVIDE || temp.type == LOAD) {
			insertSymTable(temp.a);
		}
	}
	curMap[sizeOfFunc] = offset;
	totMap[curLabel] = curMap;
	totParaMap[curLabel] = curParaMap;
}

void dataSegment() {//generate .data codes
	mprint(".data");
	int i = 0;
	for (i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == LABEL)// avoid duplicate label names
			labNameSet.insert(temp.a);
	}
	for (i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == PRINTF && temp.a != "") {
			string str = getLabName(temp.a);
		}
	}
	//generate global vars

	int offset = 0;
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == VARDEF) {
			offset++;
		}
		else if (temp.type == LABEL && temp.b == "func") {
			break;
		}
		else if (temp.type == ARRAYDEF) {
			if (isIdenfr(temp.a)) {//a must be idenfiers
				offset += atoi(temp.b.c_str());
			}
		}
	}
	mprint(globalvarLabel + ":" + ".space " + to_string(offset * 4));

	//generate .asciiz
	map<string, string>::iterator iter;
	for (iter = dataMap.begin(); iter != dataMap.end(); iter++) {
		mprint(iter->second + ":.asciiz" + "\"" + iter->first + "\"");
	}
	mprint(eof + ":.asciiz \"\\n\"");
}

bool isIdenfr(string name) {
	if (name == "")
		return false;
	if (isdigit(name[0])||name[0] == '-' || name[0] == '+')
		return false;
	return true;
}

int checkIdenfr(string a, string label) {//check and return idenfiers' value
	int ret = -1;
	if (totMap[label].find(a) != totMap[label].end())
		ret = totMap[label][a];
	return ret;
}



void genAlgebra(imTok temp,string currentlab,int line) {
	mprintc(temp.type, temp.a, temp.b, temp.c);
	string r0, r1, r2;
	r1 = loadReg(temp.b, currentlab, 0, line);
	r2 = loadReg(temp.c, currentlab, 0, line);
	r0 = loadReg(temp.a, currentlab, 1, line);
	if (r1 == temp.b && r2 == temp.c) {
		r1 = "$0";
		r2 = to_string(stoi(temp.b) + stoi(temp.c));
	}

	if (r1 == temp.b && r2 != temp.c) {// div $t0 4 $t1
		if (temp.type == ADD || temp.type == MULTI) {
			string x;
			x = r1;
			r1 = r2;
			r2 = x;
		}
		else if (temp.type == SUB) {
			mprint("li $s0," + r1);
			mprint("subu " + r0 + ",$s0," + r2);
			return;
		}
		else if (temp.type == DIVIDE) {
			mprint("li $s0," + r1);
			mprint("div " + r0 + ",$s0," + r2);
			return;
		}
	}
	if (temp.type == ADD) {
		mprint("addu " + r0 + "," + r1 + "," + r2);
	}
	else if (temp.type == SUB) {
		mprint("subu " + r0 + "," + r1 + "," + r2);
	}
	else if (temp.type == MULTI) {
		mprint("mul " + r0 + "," + r1 + "," + r2);
	}

	else if (temp.type == DIVIDE) {
		mprint("div " + r0 + "," + r1 + "," + r2);
	}
}

void genPrint(imTok temp, string currentlab,int line) {
	mprintc(temp.type, temp.a, temp.b, temp.c);

	if (temp.c == to_string(2)) {//string only
		mprint("move $s1,$a0");
		mprint("la $a0," + dataMap[temp.a]);
		mprint("li $v0,4");
		mprint("syscall");
		mprint("la $a0," + eof);
		mprint("li $v0,4");
		mprint("syscall");
		mprint("move $a0,$s1");
	}
	else if (temp.c == to_string(0)) {//int expression
		if (temp.a != "") {//string
			mprint("move $s1,$a0");
			mprint("la $a0," + dataMap[temp.a]);
			mprint("li $v0,4");
			mprint("syscall");
			mprint("move $a0,$s1");
		}
		string r1;
		r1 = loadReg(temp.b, currentlab, 0, line);//expr
		mprint("move $s1,$a0");
		if (isIdenfr(temp.b)) {
			mprint("move $a0," + r1);
		}
		else
			mprint("li $a0," + temp.b);
		mprint("li $v0,1");
		mprint("syscall");
		mprint("la $a0," + eof);
		mprint("li $v0,4");
		mprint("syscall");
		mprint("move $a0,$s1");
	}
	else if (temp.c == to_string(1)) {// char expression
		if (temp.a != "") {//string
			mprint("move $s1,$a0");
			mprint("la $a0," + dataMap[temp.a]);
			mprint("li $v0,4");
			mprint("syscall");
			mprint("move $a0,$s1");
		}
		string r1;
		r1 = loadReg(temp.b, currentlab, 0, line);//expr
		mprint("move $s1,$a0");
		if (isIdenfr(temp.b)) {
			mprint("move $a0," + r1);
		}
		else
			mprint("li $a0," + temp.b);
		mprint("li $v0,11");
		mprint("syscall");
		mprint("la $a0," + eof);
		mprint("li $v0,4");
		mprint("syscall");
		mprint("move $a0,$s1");
	}

}

void genScanf(imTok temp, string currentlab,int line) {
	mprintc(temp.type, temp.a, temp.b, temp.c);
	if (temp.b == to_string(INT)) {
		mprint("li $v0,5");
		mprint("syscall");
		string r = loadReg(temp.a, currentlab, 1, line);//expr
		mprint("move " + r + ",$v0");
	}
	if (temp.b == to_string(CHAR)) {
		mprint("li $v0,12");
		mprint("syscall");
		string r = loadReg(temp.a, currentlab, 1, line);//expr
		mprint("move " + r + ",$v0");
	}
}

void genBranch(imTok temp, string currentlab,int line) {
	mprintc(temp.type, temp.a, temp.b, temp.c);
	string r0 = loadReg(temp.a, currentlab,0,line);
	string r1 = loadReg(temp.b, currentlab, 0, line);
	saveReg(currentlab, line,0);
	mprint(imoutput[temp.type] + " " + r0 + "," + r1 + "," + temp.c);
}

void genLoad(imTok temp, string currentlab,int line) { //x = s[]
	mprintc(temp.type, temp.a, temp.b, temp.c);
	string r1 = loadReg(temp.b, currentlab, 0, line);//offset
	if (isIdenfr(temp.c)) {//base addr of array (can't be parameters)
		int s = checkIdenfr(temp.c, currentlab);
		if (s != -1) {
			mprint("addu $s0,"+ r1 + "," + to_string(s));//s[i]
			mprint("sll $s0,$s0,2");//shift by 2
			mprint("subu $s0,$sp,$s0");//real addr
		}
		else {
			s = checkIdenfr(temp.c, prefix + globalstr);
			mprint("la $s1," + globalvarLabel + "+" + to_string(s * 4));//glb addr
			mprint("sll $s0," + r1 + ",2");//offset shift by 2
			mprint("addu $s0,$s1,$s0");//real addr
		}
	}
	string r2 = loadReg(temp.a, currentlab, 1, line);//dst reg
	mprint("lw " + r2 + ",($s0)");
}

void genStore(imTok temp, string currentlab,int line) { //s[] = x
	mprintc(temp.type, temp.a, temp.b, temp.c);
	string r1 = loadReg(temp.b, currentlab, 0, line);//offset
	if (isIdenfr(temp.c)) {//base addr of array(can't be parameters)
		int s = checkIdenfr(temp.c, currentlab);
		if (s != -1) {
			mprint("addu $s0," + r1 + "," + to_string(s));//s[i]
			mprint("sll $s0,$s0,2");//offset shift by 2
			mprint("subu $s0,$sp,$s0");//real addr
		}
		else {
			s = checkIdenfr(temp.c, prefix + globalstr);
			mprint("la $s1," + globalvarLabel + "+" + to_string(s * 4));//glb addr
			mprint("sll $s0," + r1 + ",2");//offset shift by 2
			mprint("addu $s0,$s1,$s0");//real addr
		}
	}

	string r2 = loadReg(temp.a, currentlab, 0, line);// src reg
	mprint("sw " + r2 + ",($s0)");

	//mprint("sw $t2,($t1)");

}

void genMipsCode() {
	mprint(".text");
	mprint("j MAIN");
	string currentlab = "";
	initReg();
	int i = 0;
	vector<string> paras;
	for (i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == ADD || temp.type == SUB || temp.type == MULTI || temp.type == DIVIDE)
			genAlgebra(temp, currentlab,i);
		else if (temp.type == LABEL) {
			if (temp.b == "func")
				currentlab = temp.a;
			saveReg(currentlab, i, 0);
			mprint(temp.a + ":");
		}
		else if (temp.type == PRINTF) {
			genPrint(temp,currentlab,i);
		}
		else if (temp.type == SCANF) {
			genScanf(temp, currentlab,i);
		}
		else if (temp.type >= BLE && temp.type <= BEQ) {
			genBranch(temp, currentlab,i);
		}
		else if (temp.type == GOTO) {
			saveReg(currentlab, i,0);
			mprint("j " + temp.a);
		}
		else if (temp.type == LOAD) {
			genLoad(temp, currentlab,i);
		}
		else if (temp.type == STORE) {
			genStore(temp, currentlab,i);
		}
		else if (temp.type == PUSH) {
			paras.clear();
			for (int j = i; j < imList.size(); j++) {
				imTok temp = imList[j];
				if (temp.type != PUSH) {
					i = j - 1;
					break;
				}
				else {
					paras.push_back(temp.a);//  push parameters in normal order
				}
			}
		}
		else if (temp.type == CALL) {
			mprintc(CALL, temp.a);
			int funcSize = 4 * totMap[currentlab][sizeOfFunc];
			//save parameters
			for (int k = 0; k < totParaMap[currentlab].size(); k++) {
				if (k <= 3) {
					string pregs = "$a" + to_string(k);
					mprint("sw " + pregs + ",-" + to_string(k * 4) + "($sp)");
				}
			}
			//set paras
			for (int k = 0; k < paras.size(); k++) {
				mprintc(PUSH, paras[k]);
				if (k <= 3) {
					string pregs = "$a" + to_string(k);
					setReg(paras[k], pregs, currentlab);
				}
				else {
					setReg(paras[k], "$s0", currentlab);
					mprint("sw $s0,-" + to_string(funcSize + 4 + k * 4) + "($sp)");
				}
			}
			//save scene
			saveReg(currentlab, i, paras.size());

			mprint("subu $sp,$sp," + to_string(funcSize));
			mprint("sw $ra,($sp)");
			mprint("subu $sp,$sp,4");
			//jump
			mprint("jal " + temp.a);
			//recover
			mprint("addu $sp,$sp,4");
			mprint("lw $ra,($sp)");
			mprint("addu $sp,$sp," + to_string(funcSize));

			for (int k = 0; k < totParaMap[currentlab].size(); k++) {
				if (k <= 3) {
					string pregs = "$a" + to_string(k);
					mprint("lw " + pregs + ",-" + to_string(k * 4) + "($sp)");
				}
			}

			paras.clear();
		}

		else if (temp.type == RET) {
			mprintc(RET, temp.a);
			if (temp.a != "") {
				setReg(temp.a, "$v0", currentlab);
			}
			saveReg(currentlab, i, 0);
			if (currentlab != "MAIN")
				mprint("jr $ra");
			else
				mprint("j end");
			
		}
	}
	mprint("end:");
}



void genMips() {
	//printf("\nnow mips----------\n\n");
	escapedChar();
	constReplace();
	peepHoleNaive();
	blockDivision();
	//if (PRINT_IMCODE)
	//printImcode();

	if (!PRINT_TO_STD)
		freopen("mips.txt", "w", stdout);
	//printHeader();
	dataSegment();
	setSymTable();
	//printMap();
	

	genMipsCode();
}


//optimize
vector<vector<imTok>> blocksList;
void blockDivision() {
	vector<imTok> blk;
	vector<int> blkMark;
	int i;
	for (i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (temp.type == PBEGIN || temp.type == LABEL) {
			blkMark.push_back(1);
		}
		else if (i > 0 && ((imList[i-1].type >= BLE && imList[i-1].type <= BEQ) || imList[i - 1].type == GOTO)) {
			blkMark.push_back(1);
		}
		else {
			blkMark.push_back(0);
		}
	}
	for (i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		if (blkMark[i] == 0)
			blk.push_back(temp);
		else {
			if (blk.size() != 0)
				blocksList.push_back(blk);
			blk.clear();
			blk.push_back(temp);
		}
	}
	if (blk.size() != 0)
		blocksList.push_back(blk);

	//printf("\n");
	//for (i = 0; i < blocksList.size(); i++) {
	//	printf("block %d\n",i);
	//	for (int j = 0; j < blocksList[i].size(); j++) {
	//		imTok temp = blocksList[i][j];
	//		printf("%s,%s,%s,%s\n", imoutput[temp.type].c_str(), temp.a.c_str(), temp.b.c_str(), temp.c.c_str());
	//	}
	//}
	//printf("\n");
}
			
int xp = 1;
void peepHoleNaive() {// to optimize assign statements
	vector<int> flag;
	vector<imTok> newlist;
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		string a = temp.a;
		if (temp.type == ADD) {
			int j;
			string r;
			if (temp.a[0] != '@') {
				flag.push_back(1);
				continue;
			}

			if (temp.b == "0")
				r = temp.c;
			else if (temp.c == "0")
				r = temp.b;
			else {
				flag.push_back(1);
				continue;
			}
			for (j = 0; j < imList.size(); j++) {//check if "a" is changed or used in all instrs
				imTok tempj = imList[j];
				if (j == i)
					continue;
				if (tempj.a == a /*&& tempj.type != PUSH*/) {
					break;
				}
				if (tempj.b == a || tempj.c == a) {
					if (!(tempj.type == ADD || tempj.type == SUB || tempj.type == MULTI || tempj.type == DIVIDE /*|| tempj.type == PRINTF*/))
						break;
				}
			}
			if (j != imList.size()) {
				flag.push_back(1);
				continue;
			}
			for (j = i + 1; j < imList.size(); j++) {
				imTok tempj = imList[j];
				if (tempj.b == a) {
					tempj.b = r;
				}
				if (tempj.c == a) {
					tempj.c = r;
				}
				imList[j] = tempj;
			}
			flag.push_back(0);
		}
		else {
			flag.push_back(1);
		}
	}
	for (int i = 0; i < flag.size(); i++) {
		if (flag[i] == 1) {
			newlist.push_back(imList[i]);
		}
	}
	if (imList.size() != newlist.size()) {
		imList = newlist;
		//printf("\n\n\n\nnow %d peep hole\n\n\n\n", xp++);
		peepHoleNaive();
	}
}



//debug

//void printHeader() {
//	//mprint(".data");
//}

void printMap() {//debugging funcs
	mprint("\n\nnow const map printing ....");
	map<string, map<string, string>>::iterator iter1;
	map<string, string>::iterator iter2;
	for (iter1 = totConstMap.begin(); iter1 != totConstMap.end(); iter1++) {
		mprint("\nlabel = " + iter1->first);
		for (iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
			mprint("key=" + iter2->first + "  val = " + iter2->second);
		}
	}
	{
		mprint("\n\nnow var map printing ....");
		map<string, map<string, int>>::iterator iter1;
		map<string, int>::iterator iter2;
		for (iter1 = totMap.begin(); iter1 != totMap.end(); iter1++) {
			mprint("\nlabel = " + iter1->first);
			for (iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
				mprint("key=" + iter2->first + "  val = " + to_string(iter2->second));
			}
		}
	}
	{
		mprint("\n\nnow para map printing ....");
		map<string, map<string, int>>::iterator iter1;
		map<string, int>::iterator iter2;
		for (iter1 = totParaMap.begin(); iter1 != totParaMap.end(); iter1++) {
			mprint("\nlabel = " + iter1->first);
			for (iter2 = iter1->second.begin(); iter2 != iter1->second.end(); iter2++) {
				mprint("key=" + iter2->first + "  val = " + to_string(iter2->second));
			}
		}
	}
}

void printImcode() {//debugging funcs
	printf("\n\n\nafter optimize--\n\n");
	for (int i = 0; i < imList.size(); i++) {
		imTok temp = imList[i];
		printf("%s,%s,%s,%s\n", imoutput[temp.type].c_str(),temp.a.c_str(), temp.b.c_str(), temp.c.c_str());
	}
	printf("\n\n");
}