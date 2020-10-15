#pragma once
#include<iostream>
#include<vector>
#include<stdio.h>
#include<ctype.h>
#include<string>
using namespace std;
enum SYMBOL
{
	CONSTTK, INTTK, CHARTK, VOIDTK, MAINTK, IFTK, ELSETK, DOTK, WHILETK,
	FORTK, SCANFTK, PRINTFTK, RETURNTK, IDENFR, INTCON, CHARCON, STRCON,
	PLUS, MINU, MULT, DIV, GRE, GEQ, LSS, LEQ,  EQL, NEQ, ASSIGN, SEMICN,
	COMMA, LPARENT, RPARENT, LBRACK, RBRACK, LBRACE, RBRACE, SPACE, END = -1
};

#define RESLEN 13
#define OUTPUTLEN 37

const string reserve[RESLEN] = { "const","int","char","void","main","if","else","do","while","for","scanf","printf","return" };

const string output[OUTPUTLEN] = { "CONSTTK","INTTK","CHARTK","VOIDTK","MAINTK","IFTK",
	"ELSETK","DOTK","WHILETK","FORTK","SCANFTK","PRINTFTK","RETURNTK","IDENFR",
	"INTCON","CHARCON","STRCON","PLUS","MINU","MULT","DIV","GRE","GEQ","LSS","LEQ",
	"EQL","NEQ","ASSIGN","SEMICN","COMMA","LPARENT","RPARENT","LBRACK","RBRACK",
	"LBRACE","RBRACE","SPACE" };

extern int errorFlag;
extern FILE* file;
extern int line;
extern char cur;
extern string tok;
extern int tok_index;
void nextchar();
void retract();
bool isLetter();
int isRes();
int isOp();
bool isColon();
bool isEqu();
bool isGre();
bool isNot();
bool isLes();
int getsym();
void clearToken();

extern vector<pair<int, string>> tokensList;
extern vector<int> lineList;

void eprint(string s);
void eprint(char s);
void eprint(int s);
void eprint(int s,int x);
