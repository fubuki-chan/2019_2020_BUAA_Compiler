#pragma once
#include"syntax.h"
const string imoutput[30] = { "ADD", "SUB", "MULTI","DIVIDE", "RET", "PARA", "PUSH", "BLE", "BLT", "BGE", "BGT", "BNE", "BEQ",
"GOTO", "CALL", "SCANF", "PRINTF","LABEL","CONSTDEF","ARRAYDEF", "LOAD", "STORE","VARDEF","PBEGIN","PEND" };

enum midType
{
	ADD, SUB, MULTI, DIVIDE, RET, PARA, PUSH, BLE, BLT, BGE, BGT, BNE, BEQ,
	GOTO, CALL, SCANF, PRINTF, LABEL, CONSTDEF, ARRAYDEF, LOAD/*load dst,offset,array*/, STORE/*store src,offset,array*/,
	VARDEF,PBEGIN,PEND
};
enum printType
{
	PINT,PCHAR,PSTR
};
struct imTok {
	midType type;
	string a;
	string b;
	string c;
};
//debugging macros
#define PRINT_TO_STD 0
#define PRINT_IMCODE 0
#define COMMENT_ON 1
extern vector<imTok> imList;

string symbolGenerator();
string symbolGenerator(string name);
void imGenerate(midType type, string a = "", string b = "", string c = "");
string labelGenerator();



