#include"lexical.h"
#include"error.h"
#define DEBUG 1
FILE* file;

char cur;
string tok;
int tok_index = 0;
vector<pair<int, string>> tokensList;
vector<int> lineList;
int line = 1;
int errorFlag = 0;

void eprint(string s) {
	if (DEBUG)
		printf("%d %s\n", line, s.c_str());
}
void eprint(char s) {
	if (DEBUG)
		printf("%d %c\n", line, s);
}
void eprint(int s) {
	errorFlag = 1;
	if (DEBUG)
		printf("%d %c\n", line, s);
}
void eprint(int s,int x) {
	if (DEBUG)
		printf("%d %c\n", x, s);
}

void nextchar() {
	cur = fgetc(file);
	return;
}

void retract() {
	if (cur == EOF)
		return;
	if (fseek(file, -1, SEEK_CUR) != 0) {
		printf("FSEEK ERROR\n");
		exit(-1);
	}
	/*cur = fgetc(file);*/
	return;
}

void clearTok() {
	tok = "";
	tok_index = 0;
}

bool isLetter() {
	return isalpha(cur) || cur == '_';
}

int isRes() {
	for (int i = 0; i < RESLEN; i++) {
		if (tok.compare(reserve[i]) == 0)
			return i;
	}
	return -1;
}

int isOp() {
	switch (cur)
	{
		case '+':
			return PLUS;
		case '-':
			return MINU;
		case '*':
			return MULT;
		case '/':
			return DIV;
		case '(':
			return LPARENT;
		case ')':
			return RPARENT;
		case '[':
			return LBRACK;
		case ']':
			return RBRACK;
		case '{':
			return LBRACE;
		case '}':
			return RBRACE;
		case ';':
			return SEMICN;
		case ',':
			return COMMA;
		default:
			return 0;
	}
}

bool isColon() {
	return cur == ';';
}

bool isEqu() {
	return cur == '=';
}

bool isGre() {
	return cur == '>';
}

bool isNot() {
	return cur == '!';
}

bool isLes() {
	return cur == '<';
}

bool isApos() {
	return cur == '\'';
}

bool isQuo() {
	return cur == '\"';
}

bool isSpace() {
	return isspace(cur);
}

bool isNum() {
	return cur >= '0' && cur <= '9';
}

bool isChar() {
	return isNum()||(isOp()>=PLUS&&isOp()<=DIV)||isLetter();
}

bool isStringChar() {
	return (cur >= 35 && cur <= 126) || cur == 32 || cur == 33;
}

int getsym() {
	clearTok();
	nextchar();

	while (isSpace()) {
		if (cur == '\n') {
			line++;
			//printf("%d\n", line);
		}
		nextchar();
	}
	if (cur == EOF) {
		return -1;
	}
	if (isLetter()) {
		tok = string(1, cur);
		nextchar();
		while (isLetter()||isNum()) {
			tok += cur;
			nextchar();
		}
		retract();
		int index = isRes();
		if (index != -1) {
			//printf("%s %s\n", output[index].c_str(), tok.c_str());
			tokensList.push_back(make_pair(index, tok));
			lineList.push_back(line);
			clearTok();
			return index;
		}
		//printf("tok = %s\n",tok.c_str());
		else {
			//printf("%s %s\n", output[IDENFR].c_str(), tok.c_str());
			tokensList.push_back(make_pair(IDENFR, tok));
			lineList.push_back(line);
			clearTok();
			return IDENFR;
		}
	}

	else if (isNum()) {
		tok = string(1, cur);
		if (cur != '0') {
			nextchar();
			while (isNum()) {
				//printf("tok = %s\n",tok.c_str());
				tok += cur;
				nextchar();
			}
			retract();
		}
		//int num = atoi(tok.c_str());
		//printf("%s %d\n", output[INTCON].c_str(), num);
		tokensList.push_back(make_pair(INTCON, tok));
		lineList.push_back(line);
		clearTok();
		return INTCON;
	}
	else if (isOp()) {
		int index = isOp();
		//printf("%s %c\n", output[index].c_str(), cur);
		tok = string(1, cur);
		tokensList.push_back(make_pair(index, tok));
		lineList.push_back(line);
		clearTok();
		return index;
	}
	else if (isEqu()) {
		tok = string(1, cur);
		nextchar();
		if (cur == '=') {
			tok += cur;
			//printf("%s %s\n", output[EQL].c_str(), tok.c_str());
			tokensList.push_back(make_pair(EQL, tok));
			lineList.push_back(line);
			return EQL;
		}
		else {
			retract();
			//printf("%s %s\n", output[ASSIGN].c_str(), tok.c_str());
			tokensList.push_back(make_pair(ASSIGN, tok));
			lineList.push_back(line);
			return ASSIGN;
		}
	}

	else if (isLes()) {
		tok = string(1, cur);
		nextchar();
		if (cur == '=') {
			tok += cur;
			//printf("%s %s\n", output[LEQ].c_str(), tok.c_str());
			tokensList.push_back(make_pair(LEQ, tok));
			lineList.push_back(line);
			return LEQ;
		}
		else {
			retract();
			//printf("%s %s\n", output[LSS].c_str(), tok.c_str());
			tokensList.push_back(make_pair(LSS, tok));
			lineList.push_back(line);
			return LSS;
		}
	}

	else if (isGre()) {
		tok = string(1, cur);
		nextchar();
		if (cur == '=') {
			tok += cur;
			//printf("%s %s\n", output[GEQ].c_str(), tok.c_str());
			tokensList.push_back(make_pair(GEQ, tok));
			lineList.push_back(line);
			return GEQ;
		}
		else {
			retract();
			//printf("%s %s\n", output[GRE].c_str(), tok.c_str());
			tokensList.push_back(make_pair(GRE, tok));
			lineList.push_back(line);
			return GRE;
		}
	}

	else if (isNot()) {
		tok = string(1, cur);
		nextchar();
		if (cur == '=') {
			tok += cur;
			//printf("%s %s\n", output[NEQ].c_str(), tok.c_str());
			tokensList.push_back(make_pair(NEQ, tok));
			lineList.push_back(line);
			return NEQ;
		}
		else {
			return -1;
		}
	}

	else if (isApos()) {
		tok = string();
		nextchar();
		if (isApos()){
			eprint(ILLEXI);
			tokensList.push_back(make_pair(CHARCON, tok));
			lineList.push_back(line);
			return CHARCON;
		}
		if (isChar()) {
			tok += cur;
			tokensList.push_back(make_pair(CHARCON, tok));
			lineList.push_back(line);
			nextchar();
			if (!isApos()) {
				eprint(ILLEXI);
				retract();
			}
			return CHARCON;
		}
		else {
			eprint(ILLEXI);
			tokensList.push_back(make_pair(CHARCON, tok));
			lineList.push_back(line);
			nextchar();
			if (!isApos()) {
				retract();
			}
			return CHARCON;
		}
		//printf("%s %s\n", output[CHARCON].c_str(), tok.c_str());
	}

	else if (isQuo()) {
		tok = string();
		nextchar();
		while (isStringChar()) {
			tok += cur;
			nextchar();
		}
		if (!isQuo()) {
			eprint(ILLEXI);
			retract();
		}
		//printf("%s %s\n", output[STRCON].c_str(), tok.c_str());
		tokensList.push_back(make_pair(STRCON, tok));
		lineList.push_back(line);
		return STRCON;
	}
	
	else {
		eprint(ILLEXI);
		return 0;
	}

}

//--------------------------lexical---------------------------------
//--------------------------syntax---------------------------------






