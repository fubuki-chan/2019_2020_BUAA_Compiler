#pragma once
#include"midGen.h"
void dataSegment();
void genMips();
string getLabName(string name);
void mprint(string s);
void mprintc(midType type, string a = "", string b = "", string c = "");
void stackAlloc();
void getConstVal(string label, string id, string& in);
void constReplace();
void insertSymTable(string a);
void setSymTable();
void printMap();
void printImcode();
bool isIdenfr(string name);
int checkIdenfr(string a, string label);
void genMipsCode();
void printHeader();
//optimize
void peepHoleNaive();
void blockDivision();
void resetReg();
string getReg(string id,string label,int write,int line);