%{
#include <set>
#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <memory>
#include <cstring>
#include "color.h"
#include "throw.h"

#define yylex  selectsetlex
using namespace std;


static Color* ROOT=NULL;

extern int parsecolorlex(); 
#define yylex parsecolorlex

void yyerror (char *s)
 	{
 	THROW("Parsing error :\""<< (const char*)s<< "\"");
 	}

%}

%error-verbose

%union	{
	uint8_t d;
	float f;
	Color* c;
	}

%token LEX_COMMA
%token <c> LEX_COLOR
%token <d> LEX_UINT8
%token <f> LEX_FLOAT
%type <c> color input
%type <f> dec
%type <d> int8
%token LEX_RGB LEX_OPAR LEX_CPAR LEX_ERROR

%start input
%% 

input	: color
	{
	$$=$1;
	ROOT=$1;
	}
	;

color	: LEX_RGB LEX_OPAR int8 LEX_COMMA int8 LEX_COMMA int8 LEX_CPAR
	{
	$$=new Color($3,$5,$7);
	}
  	| LEX_RGB LEX_OPAR int8 LEX_COMMA int8 LEX_COMMA int8 LEX_COMMA int8 LEX_CPAR
	{
	$$=new Color($3,$5,$7,$9);
	}
	| LEX_RGB LEX_OPAR dec LEX_COMMA dec LEX_COMMA dec LEX_CPAR
	{
	$$=new Color($3,$5,$7);
	}
  	| LEX_RGB LEX_OPAR dec LEX_COMMA dec LEX_COMMA dec LEX_COMMA dec LEX_CPAR
	{
	$$=new Color($3,$5,$7,$9);
	}
	| LEX_RGB LEX_OPAR dec LEX_CPAR
	{
	$$=new Color($3);
	}
	| LEX_COLOR
	{
	$$=$1;
	}
	| error
	{
	$$=NULL;
	}
	;
	
int8	:LEX_UINT8
	{
	$$=$1;
	};
dec	:LEX_FLOAT
	{
	$$=$1;
	};
%%


void parse_color_str(Color* c,const char* str)
	{
	extern int parsecolor_scan_string(const char *);
	ROOT=NULL;
	parsecolor_scan_string(str);
	yyparse();
	if(ROOT==NULL)
		{
		THROW("Cannot parse color:"<<str);
		}
	std::memcpy((void*)c,(void*)ROOT,sizeof(Color));
	ROOT=NULL;
	}

