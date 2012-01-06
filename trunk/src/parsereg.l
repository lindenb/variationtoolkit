%{
#include <string>
#include <vector>
#include <memory>
#include <stdint.h>
#include "segments.h"
#include "segparser.tab.h"
#include "throw.h"
using namespace std;
%}

%option noyywrap

digit [0-9]
digits {digit}+
%%


[bB]([pP])?	return SIZE_BP;
[kK][bB]	return SIZE_KP;
[mM][bB]	return SIZE_MP;
[gG][bB]	return SIZE_GP;

[a-zA-Z_][a-zA-Z_0-9]* {yylval.s=new string(yytext,yyleng);return TEXT;}

{digits}(,{digits})* {
	char* p2;
	int i=0,j=0;
	for(i=0;i<yyleng;++i)
		{
		if(yytext[i]==',') continue;
		yytext[j++]=yytext[i];
		}
	yytext[j]=0;
	yylval.l=strtol(yytext,&p2,10);
	if(*p2!=0 || yylval.l<0)
		{
		THROW("Bad integer " << yytext);
		}
	return INTEGER;
	}

\:	return COLON;
\-	return DASH;
\+	return PLUS;
[\n ;]+ return DELIM;
.	THROW("Illegal character " << yytext);  

%%


