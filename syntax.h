#pragma once
#include"lexical.h"
#include"error.h"
#include<set>
#include<map>
#include<assert.h>

#define MAXIDENFRLEN 20
//struct
struct vari {
	//string name;
	symType type;
	int valInt;
	char valChar;
};

struct func {
	symType rettype;
	vector<int> paras;
};
//

//tables
extern pair<int, string> curToken;
extern int curTokenIndex;
extern map<string, vari> gloSymTable;
extern map<string, vari> tmpSymTable;
extern map<string, func> funcRetTable;
extern map<string, func> funcNonRetTable;

//function

	//TOKENS
int getTok();
void retTok();
int peep(int n);
void nextTok();
void clearToken();

	//table operations
void insertRetFunc(symType t, vector<int> para, string name);
void insertVoidFunc(vector<int> para, string name);
void insertTable(symType t, string name);
int checkTable(string name);
vector<int> checkFuncTable(string name, map<string, func> funcmap, int* p);
void printTable(map<string, vari> tmp);
int checkCertainTable(string name);

	//non terminators
//int expression();
//int term();
//int factor();
int expression(string& a);
int term(string& a);
int factor(string& a);

void constDefi();
void constDecl();
void integ();
void varDefi();
void varDecl();
pair<string, symType> headOfDeclare();
void funcRetval();
vector<int> parameter();
int multiStms(int rettype);
void funcNonRetval();
void mainfunc();
int stm(int rettype);
int stmLines(int rettype);
void assignStm();
int ifStm(int rettype);
string ifJudge();
int loopStm(int rettype);
int funcRetCall();
int funcNonRetCall();
vector<int> paraVal();
void scanStm();
void printStm();
int retStm(int rettype);
void step();
void program();
void strs();


// intermediate code
void genGlobalVar();