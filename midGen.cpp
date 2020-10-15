#include"midGen.h"

using namespace std;
vector<imTok> imList;
int symGenCnt = 0;
string symbolGenerator() {
	string temp = "@t";
	temp += to_string(symGenCnt++);
	//while (checkTable(temp) != -1) {
	//	temp += to_string(symGenCnt++);
	//}
	return temp;
}

int lbGenCnt = 0;
string labelGenerator() {
	string temp = "_label___";
	temp += to_string(lbGenCnt++);
	//while (checkTable(temp) != -1) {
	//	temp += to_string(lbGenCnt++);
	//}
	return temp;
}

string symbolGenerator(string name) {
	string temp = "t";
	temp += to_string(symGenCnt++);
	int x = checkCertainTable(name);
	if (x <= 0) {
		return name;
	}
	while (checkTable(temp) != -1) {
		temp += to_string(symGenCnt++);
	}
	return temp;
}

void imGenerate(midType type, string a, string b, string c) {
	if (errorFlag) {
		imList.clear();
		return;
	}
	imTok temp;
	temp.type = type;
	temp.a = a;
	temp.b = b;
	temp.c = c;
	imList.push_back(temp);
	if (PRINT_IMCODE)
		printf("%s,%s,%s,%s\n", imoutput[type].c_str(), a.c_str(), b.c_str(), c.c_str());
}


