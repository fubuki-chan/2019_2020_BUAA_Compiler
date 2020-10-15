#include"midGen.h"

map<string, vari> gloSymTable;
map<string, vari> tmpSymTable;
map<string, func> funcRetTable;
map<string, func> funcNonRetTable;

pair<int, string> curToken;
int curTokenIndex = 0;

int imControl = 1;

void insertRetFunc(symType t, vector<int> para, string name) {
	map<string, func>::iterator iter = funcRetTable.find(name);

	if (iter != funcRetTable.end()) {
		if (iter->second.paras == para) {
			eprint(DUPNAME);
			return;
		}
	}

	func temp;
	temp.rettype = t;
	temp.paras = para;
	funcRetTable[name] = temp;
}

void insertVoidFunc(vector<int> para, string name) {
	map<string, func>::iterator iter = funcNonRetTable.find(name);
	if (iter != funcNonRetTable.end()) {
		if (iter->second.paras == para) {
			eprint(DUPNAME);
			return;
		}
	}
	func temp;
	temp.paras = para;
	funcNonRetTable[name] = temp;
}


void insertTable(symType t, string name) {
	map<string, vari>::iterator iter = tmpSymTable.find(name);
	if (iter != tmpSymTable.end()) {
		eprint(DUPNAME);
	}
	else {
		vari temp;
		temp.type = t;
		tmpSymTable[name] = temp;
		if (t == INT && imControl)
			imGenerate(VARDEF, "int", name);
		if (t == CHAR && imControl)
			imGenerate(VARDEF, "int", name);
	}
}

int checkTable(string name) {//attention: in expression(), undefined names will be recognised as chars
	map<string, vari>::iterator iter = tmpSymTable.find(name);
	map<string, vari>::iterator iter2 = gloSymTable.find(name);
	if (iter == tmpSymTable.end() && iter2 == gloSymTable.end()) {
		eprint(UNDNAME);
		return -1;
	}
	else if (iter != tmpSymTable.end()) {
		return iter->second.type;
	}
	else if (iter2 != gloSymTable.end()) {
		return iter2->second.type;
	}
	return -1;
}

int checkCertainTable(string name) {//attention: in expression(), undefined names will be recognised as chars
	map<string, vari>::iterator iter = tmpSymTable.find(name);
	if (iter == tmpSymTable.end()) {
		return -1;//fine,use origin name
	}
	else {
		map<string, vari>::iterator iter2 = gloSymTable.find(name);
		if (iter2 != gloSymTable.end()) {
			return 1;
		}
		return 0;
	}
}

vector<int> checkFuncTable(string name, map<string, func> funcmap,int* p) {
	vector<int> temp;
	map<string, func>::iterator iter = funcmap.find(name);
	if (iter == funcmap.end()) {
		eprint(UNDNAME);
		return temp;
	}
	else {
		*p = iter->second.rettype;
		return iter->second.paras;
	}
}

void printTable(map<string, vari> tmp) {
	map<string, vari>::iterator iter = tmp.begin();
	for (; iter != tmp.end(); iter++)
	{
		cout << iter->first << ' ' << iter->second.type << endl;
	}
	cout << endl;
}

void print(string s) {
	//printf("<%s>\n", s.c_str());
}

void doTerminator(int i) {
	if (getTok() != i) {
		if (i == SEMICN || i == RPARENT || i == RBRACK || i == WHILETK) {
			if (i == SEMICN) {
				eprint(SEMICNERR,lineList[curTokenIndex-1]);
			}
			else if (i == RPARENT) {
				eprint(RPERR);
			}
			else if (i == RBRACK) {
				eprint(RBERR);
			}
			else if (i == WHILETK) {
				eprint(WHILEERR);
			}
			return;
		}
	}
	nextTok();
}

int getTok() {
	static int tokenListSize = tokensList.size();
	if (curTokenIndex >= tokenListSize)
		return END;
	curToken = tokensList[curTokenIndex];
	line = lineList[curTokenIndex];
	return curToken.first;
}

string getIdenfr(int i) {
	assert(i == IDENFR);
	getTok();
	return curToken.second;
}

int peep(int n) {
	//n > 0	
	static int tokenListSize = tokensList.size();
	if (curTokenIndex + n >= tokenListSize)
		return END;
	return tokensList[curTokenIndex + n].first;
}

void nextTok() {
	curTokenIndex++;
}


void retTok() {
	curTokenIndex--;
}

void clearToken() {
	curToken = make_pair(0, "");
	curTokenIndex = 0;
}

void uinteg() {
	doTerminator(INTCON);
	//print("uint");
	print("无符号整数");
}

void integ() {
	if (getTok() == PLUS || getTok() == MINU) {
		nextTok();
	}
	uinteg();
	print("整数");
}

void relation() {
	if (getTok() == GRE || getTok() == GEQ|| getTok() == LSS || getTok() == LEQ || getTok() == EQL || getTok() == NEQ) {
		nextTok();
	}
	else
		;
}

int expression(string& s) {
	int isChar = 1;
	string tempstr = symbolGenerator();
	int leadsym = 0;
	if (getTok() == PLUS || getTok() == MINU) {
		leadsym = getTok();
		nextTok();
		isChar = 0;
	}
	string a;
	isChar &= term(a);
	if (leadsym == MINU) {
		imGenerate(SUB, tempstr, "0", a);
	}
	else {
		imGenerate(ADD, tempstr, a, "0");
	}
	while (getTok() == PLUS || getTok() == MINU) {
		int sym = getTok();
		nextTok();
		string b;
		term(b);
		isChar = 0;
		imGenerate((midType)(sym - PLUS), tempstr, tempstr, b);
	}
	print("表达式");
	s = tempstr;
	return isChar;
	//print("expr");
}

int term(string& s) {
	int isChar = 1;
	string tempstr = symbolGenerator();
	string a;
	isChar &= factor(a);
	imGenerate(ADD, tempstr, a, "0");
	while (getTok() == MULT || getTok() == DIV) {
		int sym = getTok();
		nextTok();
		factor(a);
		imGenerate((midType)(sym - MULT + MULTI), tempstr, tempstr, a);
		isChar = 0;
	}
	print("项");
	//print("term");
	s = tempstr;
	return isChar;
}

int factor(string& s) {
	string tempstr = symbolGenerator();
	s = tempstr;
	int isChar = 1;
	if (getTok() == IDENFR) {
		if (peep(1) == LPARENT) {
			int k = funcRetCall();
			s = "RET";
			if (k == INT)
				isChar = 0;
		}
		else {
			string name = getIdenfr(IDENFR);
			int y = checkTable(name);
			//s = symbolGenerator(name);
			s = name;
			if (y == CONSTINT || y == INT || y == INTARRAY) {
				isChar = 0;
			}
			nextTok();
			if (getTok() == LBRACK) {
				nextTok();
				string k;
				int x = expression(k);
				if (x != INT) {
					eprint(INDEXERR);
				}
				doTerminator(RBRACK);
				imGenerate(LOAD, tempstr, k, name);
				s = tempstr;
			}
		}
	}
	else if (getTok() == LPARENT) {
		isChar = 0;
		nextTok();
		expression(s);
		doTerminator(RPARENT);
	}
	else if (getTok() == PLUS || getTok() == MINU) {
		isChar = 0;
		s = tokensList[curTokenIndex + 1].second;
		if (getTok() == MINU)
			s = "-" + s;
		integ();
	}
	else if (getTok() == INTCON) {
		isChar = 0;
		s = curToken.second;
		integ();
	}
	else if (getTok() == CHARCON) {
		int t = curToken.second[0] - '\0';
		string x = to_string(t);
		s = x;
		nextTok();
	}
	else
		;
	print("因子");
	//print("factor");
	return isChar;
}

void constDecl() {//explain
	doTerminator(CONSTTK);
	constDefi();
	doTerminator(SEMICN);
	while (getTok() == CONSTTK) {
		doTerminator(CONSTTK);
		constDefi();
		doTerminator(SEMICN);
	}
	//print("constDecl");
	print("常量说明");
}
void constDefi() {//definition
	if (getTok() == INTTK) {
		doTerminator(INTTK);
		string s = getIdenfr(IDENFR);
		doTerminator(IDENFR);
		doTerminator(ASSIGN);
		//integ();
		//
		string temp;
		if (getTok() == PLUS || getTok() == MINU) {
			temp = tokensList[curTokenIndex].second;
			nextTok();
			if (getTok() != INTCON) {
				eprint(CONSTTYPE);
				nextTok();
			}
			else {
				temp += tokensList[curTokenIndex].second;
				uinteg();
			}
		}
		else if (getTok() != INTCON) {
			eprint(CONSTTYPE);
			nextTok();
		}
		else {
			temp = tokensList[curTokenIndex].second;
			uinteg();
		}
		//
		insertTable(CONSTINT,s);
		imGenerate(CONSTDEF, "int", s, temp);
		while (getTok() == COMMA) {
			doTerminator(COMMA);
			s = getIdenfr(IDENFR);
			doTerminator(IDENFR);
			doTerminator(ASSIGN);
			/*integ();*/
			//
			if (getTok() == PLUS || getTok() == MINU) {
				temp = tokensList[curTokenIndex].second;
				nextTok();
				if (getTok() != INTCON) {
					eprint(CONSTTYPE);
					nextTok();
				}
				else {
					temp += tokensList[curTokenIndex].second;
					uinteg();
				}
			}
			else if (getTok() != INTCON) {
				eprint(CONSTTYPE);
				nextTok();
			}
			else {
				temp = tokensList[curTokenIndex].second;
				uinteg();
			}
			//
			insertTable(CONSTINT, s);
			imGenerate(CONSTDEF, "int", s, temp);
		}
	}
	else if (getTok() == CHARTK) {
		doTerminator(CHARTK);
		string s = getIdenfr(IDENFR);
		string temp;
		doTerminator(IDENFR);
		doTerminator(ASSIGN);
		if (getTok() != CHARCON) {
			eprint(CONSTTYPE);
			nextTok();
		}
		else { 
			temp = tokensList[curTokenIndex].second;
			doTerminator(CHARCON); 
		}
		insertTable(CONSTCHAR, s);
		imGenerate(CONSTDEF, "char", s, temp);
		while (getTok() == COMMA) {
			doTerminator(COMMA);
			s = getIdenfr(IDENFR);
			doTerminator(IDENFR);
			doTerminator(ASSIGN);
			if (getTok() != CHARCON) {
				eprint(CONSTTYPE);
				nextTok();
			}
			else {	
				temp = tokensList[curTokenIndex].second;
				doTerminator(CHARCON);
			}
			insertTable(CONSTCHAR, s);
			imGenerate(CONSTDEF, "char", s, temp);
		}
	}
	else
		;
	//print("constDefi");
	print("常量定义");
}

pair<string, symType> headOfDeclare() {
	string s;//name
	symType type;
	if (getTok() == INTTK) {
		doTerminator(INTTK);
		s = getIdenfr(IDENFR);
		type = INT;
		doTerminator(IDENFR);
	}
	else if (getTok() == CHARTK) {
		doTerminator(CHARTK);
		s = getIdenfr(IDENFR);
		type = CHAR;
		doTerminator(IDENFR);
	}
	else
		;
	//print("headOfDeclare");
	print("声明头部");
	return make_pair(s,type);
}

symType typeIdenfr() {
	if (getTok() == INTTK) {
		doTerminator(INTTK);
		return INT;
	}
	else if (getTok() == CHARTK) {
		doTerminator(CHARTK);
		return CHAR;
	}
	else
		;
}

void varDecl() {//explain
	varDefi();
	doTerminator(SEMICN);
	while ((getTok() == INTTK || getTok() == CHARTK) && (peep(2) == COMMA || peep(2) == LBRACK || peep(2) == SEMICN)) {
		varDefi();
		doTerminator(SEMICN);
	}
	//print("varDecl");
	print("变量说明");
}


void varDefi() {//definition
	symType x = typeIdenfr();
	string s = getIdenfr(IDENFR);
	doTerminator(IDENFR);
	if (getTok() == LBRACK) {
		doTerminator(LBRACK);
		getTok();
		//printf("---  %s", curToken.second.c_str());
		assert(atoi(curToken.second.c_str()) > 0);//TODO
		string size = curToken.second;
		uinteg();
		doTerminator(RBRACK);
		insertTable(symType(x+2), s);
		imGenerate(ARRAYDEF, s, size);
	}
	else {
		insertTable(x, s);
	}
	while (getTok() == COMMA) {
		doTerminator(COMMA);
		s = getIdenfr(IDENFR);
		doTerminator(IDENFR);

		if (getTok() == LBRACK) {
			doTerminator(LBRACK);
			getTok();
			//printf("---  %s",curToken.second.c_str());
			assert(atoi(curToken.second.c_str()) > 0);//TODO
			string size = curToken.second;
			uinteg();
			doTerminator(RBRACK);
			insertTable(symType(x + 2), s);
			imGenerate(ARRAYDEF, s, size);
		}
		else {
			insertTable(x, s);
		}
	}
	//print("varDefi");
	print("变量定义");
}

void funcRetval() {
	tmpSymTable.clear();
	pair<string, symType> s = headOfDeclare();
	imGenerate(LABEL, s.first, "func", "");
	doTerminator(LPARENT);
	vector<int> temp = parameter();
	insertRetFunc(s.second, temp, s.first);

	doTerminator(RPARENT);
	doTerminator(LBRACE);
	int rettype = s.second;
	int x = multiStms(rettype);
	if (x == 0) {
		eprint(RETUNMATCH);
	}
	doTerminator(RBRACE);
	//print("funcRetval");
	print("有返回值函数定义");
}

void funcNonRetval() {

	tmpSymTable.clear();
	doTerminator(VOIDTK);
	string s = getIdenfr(IDENFR);
	imGenerate(LABEL, s, "func", "");
	//funcNonRetList.insert(s);
	doTerminator(IDENFR);
	doTerminator(LPARENT);
	vector<int> temp = parameter();
	doTerminator(RPARENT);
	doTerminator(LBRACE);
	insertVoidFunc(temp, s);
	int rettype = -1;
	multiStms(rettype);
	imGenerate(RET);
	doTerminator(RBRACE);
	//print("funcNonRetval");
	print("无返回值函数定义");
}

vector<int> parameter() {
	vector<int> temp;
	imControl = 0;
	if (getTok() == INTTK) {
		temp.push_back(INT);
		doTerminator(INTTK);
		string s = getIdenfr(IDENFR);
		doTerminator(IDENFR);
		insertTable(INT, s);
		imGenerate(PARA, "int", s, "");
		while (getTok() == COMMA) {
			doTerminator(COMMA);
			symType x = typeIdenfr();
			temp.push_back(x);
			s = getIdenfr(IDENFR);
			doTerminator(IDENFR);
			insertTable(x, s);
			if (x == INT)
				imGenerate(PARA, "int", s, "");
			else if (x == CHAR)
				imGenerate(PARA, "char", s, "");
		}
	}
	else if (getTok() == CHARTK) {
		temp.push_back(CHAR);
		doTerminator(CHARTK);
		string s = getIdenfr(IDENFR);
		doTerminator(IDENFR);
		insertTable(CHAR, s);
		imGenerate(PARA, "char", s, "");
		while (getTok() == COMMA) {
			doTerminator(COMMA);
			symType x = typeIdenfr();
			temp.push_back(x);
			s = getIdenfr(IDENFR);
			doTerminator(IDENFR);
			insertTable(x, s);
			if (x == INT)
				imGenerate(PARA, "int", s, "");
			else if (x == CHAR)
				imGenerate(PARA, "char", s, "");
		}
	}
	imControl = 1;
	//print("parameter");
	print("参数表");
	return temp;
}

int multiStms(int rettype) {
	if (getTok() == CONSTTK) {
		constDecl();
	}
	if (getTok() == INTTK || getTok() == CHARTK) {
		varDecl();
	}
	int x = stmLines(rettype);
	//print("multiStms");
	print("复合语句");
	return x;
}

void mainfunc() {
	tmpSymTable.clear();
	doTerminator(VOIDTK);
	doTerminator(MAINTK);
	doTerminator(LPARENT);
	doTerminator(RPARENT);
	doTerminator(LBRACE);
	imGenerate(LABEL, "MAIN", "func", "");
	multiStms(-1);
	doTerminator(RBRACE);
	//print("mainfunc");
	print("主函数");
}



void assignStm() {
	string name = getIdenfr(IDENFR);
	symType x = (symType)checkTable(name);
	if (x == CONSTINT || x == CONSTCHAR) {
		eprint(CONSTASSIGN);
	}
	doTerminator(IDENFR);
	if (getTok() == ASSIGN) {
		doTerminator(ASSIGN);
		string src;
		expression(src);
		//string dst = symbolGenerator(name);
		string dst = name;
		imGenerate(ADD, dst, "0", src);
	}
	else if (getTok() == LBRACK) {
		doTerminator(LBRACK);
		string offset;
		int x = expression(offset);
		if (x != INT) {
			eprint(INDEXERR);
		}
		doTerminator(RBRACK);
		doTerminator(ASSIGN);
		string src;
		expression(src);
		imGenerate(STORE, src, offset, name);
	}
	//print("assignStm");
	print("赋值语句");
}

int ifStm(int rettype) {
	doTerminator(IFTK);
	doTerminator(LPARENT);
	string label = ifJudge();
	doTerminator(RPARENT);
	int x = 0;
	x += stm(rettype);
	string end = labelGenerator();

	if (getTok() == ELSETK) {
		imGenerate(GOTO, end, "", "");
		imGenerate(LABEL, label, "branch", "");
		doTerminator(ELSETK);
		x += stm(rettype);
		imGenerate(LABEL, end, "branch", "");
	}
	else {
		imGenerate(LABEL, label, "branch", "");
	}
	//print("ifstm");
	print("条件语句");
	return x;
}
// if condition is false , then goto return value
string ifJudge() {
	string s1, s2;
	int x = expression(s1);
	string l1 = labelGenerator();
	int y = 0;
	if (getTok() == GRE || getTok() == GEQ || getTok() == LSS || getTok() == LEQ || getTok() == EQL || getTok() == NEQ) {
		int k = getTok();
		relation();
		y = expression(s2);
		imGenerate((midType)(k -GRE + BLE), s1, s2, l1);
	}
	else {
		imGenerate(BEQ, s1, "0", l1);
	}
	//print("ifJudge");
	if (x + y != INT) {
		eprint(ILLJUDGE);
	}
	print("条件");
	return l1;
}
// if condition is false , then goto label
void ifJudge(string label) {
	string s1, s2;
	int x = expression(s1);
	string l1 = labelGenerator();
	int y = 0;
	if (getTok() == GRE || getTok() == GEQ || getTok() == LSS || getTok() == LEQ || getTok() == EQL || getTok() == NEQ) {
		int k = getTok();
		relation();
		y = expression(s2);
		imGenerate((midType)(k - GRE + BLE), s1, s2, label);
	}
	else {
		imGenerate(BEQ, s1, "0", label);
	}
	//print("ifJudge");
	if (x + y != INT) {
		eprint(ILLJUDGE);
	}
	print("条件");
}

void ifJudgeTrue(string label) {//if condition is true , then goto label
	string s1, s2;
	int x = expression(s1);
	string l1 = labelGenerator();
	int y = 0;
	if (getTok() == GRE || getTok() == GEQ || getTok() == LSS || getTok() == LEQ || getTok() == EQL || getTok() == NEQ) {
		int k = getTok();
		relation();
		y = expression(s2);
		if (k == GRE)
			imGenerate(BGT, s1, s2, label);
		if (k == GEQ)
			imGenerate(BGE, s1, s2, label);
		if (k == LSS)
			imGenerate(BLT, s1, s2, label);
		if (k == LEQ)
			imGenerate(BLE, s1, s2, label);
		if (k == EQL)
			imGenerate(BEQ, s1, s2, label);
		if (k == NEQ)
			imGenerate(BNE, s1, s2, label);

	}
	else {
		imGenerate(BNE, s1, "0", label);
	}
	//print("ifJudge");
	if (x + y != INT) {
		eprint(ILLJUDGE);
	}
	print("条件");
}

int loopStm(int rettype) {
	int x = 0;
	if (getTok() == WHILETK) {
		doTerminator(WHILETK);
		doTerminator(LPARENT);
		string begin = labelGenerator();
		imGenerate(LABEL, begin, "branch", "");
		string label = ifJudge();
		doTerminator(RPARENT);
		x = stm(rettype);
		imGenerate(GOTO, begin);
		imGenerate(LABEL, label, "branch", "");
	}
	else if (getTok() == DOTK) {
		string begin = labelGenerator();
		doTerminator(DOTK);
		imGenerate(LABEL, begin, "branch", "");
		x = stm(rettype);
		doTerminator(WHILETK);
		doTerminator(LPARENT);
		ifJudgeTrue(begin);
		doTerminator(RPARENT);
	}
	else if (getTok() == FORTK) {
		string expr;
		doTerminator(FORTK);
		doTerminator(LPARENT);
		string name = getIdenfr(IDENFR);
		checkTable(name);
		doTerminator(IDENFR);
		doTerminator(ASSIGN);
		expression(expr);
		doTerminator(SEMICN);
		imGenerate(ADD, name, expr, "0");
		string begin = labelGenerator();
		imGenerate(LABEL, begin);
		string end = labelGenerator();
		ifJudge(end);
		doTerminator(SEMICN);
		name = getIdenfr(IDENFR);
		checkTable(name);
		doTerminator(IDENFR);
		doTerminator(ASSIGN);
		string name2 = getIdenfr(IDENFR);
		checkTable(name2);
		doTerminator(IDENFR);
		int sig = PLUS;
		if (getTok() == PLUS || getTok() == MINU) {
			sig = getTok();
			nextTok();
		}
		string st = tokensList[curTokenIndex].second;
		step();
		doTerminator(RPARENT);
		x = stm(rettype);
		imGenerate((midType)(sig - PLUS + ADD), name, name2, st);
		imGenerate(GOTO, begin);
		imGenerate(LABEL, end, "branch", "");
	}
	//print("loopStm");
	print("循环语句");
	return x;
}

void step() {
	uinteg();
	//print("step");
	print("步长");
}

int funcRetCall() {
	//todo
	vector<int> s;
	string name = getIdenfr(IDENFR);
	int k = 0;
	s = checkFuncTable(name,funcRetTable,&k);
	doTerminator(IDENFR);
	doTerminator(LPARENT);
	vector<int> t = paraVal();
	imGenerate(CALL, name, "ret", "");
	if (s.size() != t.size()) {
		eprint(PARACNT);
	}
	else if (s != t) {
		eprint(PARATYPE);
	}
	doTerminator(RPARENT);
	//print("funcRetCall");
	print("有返回值函数调用语句");
	return k;
}

int funcNonRetCall() {
	//todo
	vector<int> s;
	string name = getIdenfr(IDENFR);
	int k = 0;
	s = checkFuncTable(name, funcNonRetTable,&k);
	doTerminator(IDENFR);
	doTerminator(LPARENT);
	vector<int> t = paraVal();
	imGenerate(CALL, name, "nonret", "");
	if (s.size() != t.size()) {
		eprint(PARACNT);
	}
	else if (s != t) {
		eprint(PARATYPE);
	}
	doTerminator(RPARENT);
	//print("funcNonRetCall");
	print("无返回值函数调用语句");
	return k;
}

vector<int> paraVal() {
	vector<int> s;
	string para;
	vector<string> paras;
	int temp = getTok();
	if (temp == PLUS || temp == MINU || temp == IDENFR || temp == LPARENT || temp == INTCON || temp == CHARCON) {
		s.push_back(expression(para));
		paras.push_back(para);
		//imGenerate(PUSH, para, "", "");
		while (getTok() == COMMA) {
			doTerminator(COMMA);
			string p;
			s.push_back(expression(p));
			paras.push_back(p);
			//imGenerate(PUSH, p, "", "");
		}
	}
	for (int i = 0; i < paras.size(); i++) {
		imGenerate(PUSH, paras[i]);
	}
	//print("paraval");
	print("值参数表");
	return s;
}


void scanStm() {
	doTerminator(SCANFTK);
	doTerminator(LPARENT);
	string name = getIdenfr(IDENFR);
	int type = checkTable(name);
	doTerminator(IDENFR);
	imGenerate(SCANF, name, to_string(type), "");
	while (getTok() == COMMA) {
		doTerminator(COMMA);
		string name = getIdenfr(IDENFR);
		int type = checkTable(name);
		doTerminator(IDENFR);
		imGenerate(SCANF, name, to_string(type), "");
	}
	doTerminator(RPARENT);
	//print("scanStm");
	print("读语句");
}

void printStm() { // printf 0 num 1 char 2 str
	doTerminator(PRINTFTK);
	doTerminator(LPARENT);
	int temp = getTok();
	string str = "";
	string expr = "";
	int printtype = 0;
	if (temp == STRCON) {
		printtype = 2;
		str = tokensList[curTokenIndex].second;
		strs();
		if (getTok() == COMMA) {
			doTerminator(COMMA);
			printtype = expression(expr);
		}
		doTerminator(RPARENT);
	}
	else if (temp == PLUS || temp == MINU || temp == IDENFR || temp == LPARENT || temp == INTCON || temp == CHARCON) {
		printtype = expression(expr);
		doTerminator(RPARENT);
	}
	imGenerate(PRINTF, str, expr, to_string(printtype));
	//print("printStm");
	print("写语句");
}

void strs() {
	doTerminator(STRCON);
	//print("str");
	print("字符串");
}
int retStm(int rettype) {
	doTerminator(RETURNTK);
	int x = -1;//-1 void 0 int 1 char
	string ret = "";
	if (getTok() == LPARENT) {
		doTerminator(LPARENT);
		x = expression(ret);
		doTerminator(RPARENT);
	}
	if (rettype != x && rettype == -1) {
		eprint(NRETUNMATCH);
	}
	else if (rettype != -1 && rettype != x) {
		eprint(RETUNMATCH);
	}
	//print("returnstm");
	imGenerate(RET, ret, "", "");
	print("返回语句");
	return 1;
}

int stm(int rettype) {
	int x = 0;
	if (getTok() == IFTK) {
		x = ifStm(rettype);
	}
	else if (getTok() == WHILETK || getTok() == DOTK || getTok() == FORTK) {
		x = loopStm(rettype);
	}
	else if (getTok() == LBRACE) {
		doTerminator(LBRACE);
		x = stmLines(rettype);
		doTerminator(RBRACE);
	}
	else if (getTok() == IDENFR) {
		string s = getIdenfr(IDENFR);
		if (funcNonRetTable.find(s) != funcNonRetTable.end()) {
			funcNonRetCall();
		}
		else if (funcRetTable.find(s) != funcRetTable.end()) {
			funcRetCall();
		}
		else {
			assignStm();
		}
		doTerminator(SEMICN);
		//func 
	}
	else if (getTok() == SCANFTK) {
		scanStm();
		doTerminator(SEMICN);
	}
	else if (getTok() == PRINTFTK) {
		printStm();
		doTerminator(SEMICN);
	}
	else if (getTok() == RETURNTK) {
		x = retStm(rettype);
		doTerminator(SEMICN);
	}
	else {
		doTerminator(SEMICN);
	}
	//print("stm");
	print("语句");
	return x;
}

int stmLines(int rettype) {
	int temp = getTok();
	int x = 0;
	while (temp == IFTK || temp == WHILETK || temp == LPARENT || temp == LBRACE || temp == IDENFR || temp == SCANFTK || temp == PRINTFTK || temp == RETURNTK || temp == SEMICN || getTok() == DOTK || getTok() == FORTK) {
		if (temp == RETURNTK)
			x = 1;
		x += stm(rettype);
		temp = getTok();
	}
	return x;
	//print("stmLines");
	print("语句列");
}
void program() {
	imGenerate(PBEGIN);
	if (getTok() == CONSTTK) {
		constDecl();
	}
	if (getTok() == INTTK || getTok() == CHARTK) {
		//doTerminator(IDENFR);
		if (peep(2) == COMMA || peep(2) == LBRACK || peep(2) == SEMICN) {
			varDecl();
		}
	}
	gloSymTable = tmpSymTable;
	tmpSymTable.clear();

	//genGlobalVar();

	if (getTok() == INTTK || getTok() == CHARTK || (getTok() == VOIDTK && peep(1) == IDENFR)) {
		if (getTok() == INTTK || getTok() == CHARTK) {
			funcRetval();
		}
		else if (getTok() == VOIDTK && peep(1) == IDENFR) {
			funcNonRetval();
		}
		while (getTok() == INTTK || getTok() == CHARTK || (getTok() == VOIDTK && peep(1) == IDENFR)) {
			if (getTok() == INTTK || getTok() == CHARTK) {
				funcRetval();
			}
			else if (getTok() == VOIDTK && peep(1) == IDENFR) {
				funcNonRetval();
			}
		}
	}
	mainfunc();
	//print("program");
	print("程序");
	imGenerate(PEND);
}

//void genGlobalVar() {
//	map<string, vari>::iterator iter;
//	for (iter = gloSymTable.begin(); iter != gloSymTable.end(); iter++) {
//		string name = iter->first;
//		symType type = iter->second.type;
//		if (type == INT)
//			imGenerate(GLOBALVARDEF, "int", name);
//		else if (type == CHAR)
//			imGenerate(GLOBALVARDEF, "char", name);
//	}
//}

