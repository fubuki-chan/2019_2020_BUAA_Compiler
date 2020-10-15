#include"mips.h"
int main() {
	//fopen_s(&file,"testfile5.txt", "r");
	//file = fopen("1.txt", "r");
	file = fopen("testfile.txt", "r");
	//freopen("error.txt", "w", stdout);
	while (getsym() != -1)
		;
	//clearToken();
	program();

	genMips(); 
	return 0;
}