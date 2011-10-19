%{
#include <string>
class Node;
#include "selectset.tab.h"
#include "throw.h"
using namespace std;
%}

%option noyywrap

%%
(\"[^\']*\"|\'[^\"]*\') {yylval.s=new string(&yytext[1],yyleng-2);return TEXT;}
\&\& return LEX_AND;
\|\| return LEX_OR;
\! return LEX_NOT;
[a-zA-Z0-9_]+ {yylval.s=new string(yytext,yyleng);return TEXT;}
\(	return OPAR;
\)	return CPAR;
(.|\n)	{
	if(!isspace(yytext[0]))
		{
		return yytext[0];
		}
	}  

%%
