#pragma once
#define ERRORDEBUG 0
#include<stdio.h>
#include<string>
enum ERROR
{
	ILLEXI = 'a',DUPNAME,UNDNAME,PARACNT,PARATYPE,ILLJUDGE,NRETUNMATCH,RETUNMATCH,
	INDEXERR,CONSTASSIGN,SEMICNERR,RPERR,RBERR,WHILEERR,CONSTTYPE
};
enum symType {
	INT, CHAR,INTARRAY,CHARARRAY,CONSTCHAR, CONSTINT
};
