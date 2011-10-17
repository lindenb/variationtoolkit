%{
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <stdint.h>
#include "segments.h"
#include "throw.h"
using namespace std;
extern int segparserlex(); 
#define yylex segparserlex

void yyerror (char *s)
 	{
 	THROW("Parsing error :\""<< (const char*)s<< "\"");
 	}
static std::vector<ChromStartEnd>* ROOT_VECTOR;
%}

%error-verbose
%union {
	std::string* s;
	int32_t i;
	int64_t l;
	StartEnd* se;
	ChromStartEnd* cse;
	std::vector<ChromStartEnd>* v;
	}
	


%token PLUS SIZE_BP SIZE_KP SIZE_MP SIZE_GP COLON DASH DELIM
%token <s> TEXT
%token <l> INTEGER
%type <l> factor integer pos optpos
%type <se> range
%type <s> chrom
%type <cse> segment 
%type <v> segments input
%start input
%% 

input: segments 
	{
	$$=$1;
	ROOT_VECTOR=$$;
	}

segments:segment {
	$$=new std::vector<ChromStartEnd>;
	$$->push_back(*($1));
	delete $1;
	}| segments DELIM segment
	{
	$$=$1;
	$$->push_back(*($3));
	delete $3;
	}
	;
segment: chrom range
	{
	$$=new ChromStartEnd($1->c_str(),$2->start,$2->end);
	delete $1;
	delete $2;
	if($$->start > $$->end) THROW("Range error in "<< *($$));
	}
chrom:  TEXT {$$=$1}|
	INTEGER { ostringstream os; os << $1; $$=new string(os.str()); }
	;
range: 	{$$=new StartEnd(0,numeric_limits<int32_t>::max()-10); } |
	COLON pos optpos { $$=new StartEnd((int32_t)$2,(int32_t)$3); } |
	COLON DASH  pos { $$=new StartEnd(1,(int32_t)$3); } |
	COLON pos PLUS pos { $$=new StartEnd($2-$4<1?1:(int32_t)($2-$4),(int32_t)($2+$4)); } 
	;
optpos:	DASH  pos { $$=$2;} | { $$=numeric_limits<int32_t>::max()-10;};

pos:	integer factor 
	{
	$$=$1*$2;
	if($$ > numeric_limits<int32_t>::max() ) THROW("Value is too large for bp " << $$);
	};
factor:	{$$=1;}| SIZE_BP {$$=1L;}| SIZE_KP {$$=1000L;} | SIZE_MP {$$=1000000L;} | SIZE_GP {$$=1000000000L;};
integer: INTEGER { $$=$1;};
%%

std::auto_ptr<std::vector<ChromStartEnd> > parseSegments(const char* s)
	{
	extern int segparser_scan_string(const char *);
	//extern int segparserlex_destroy(void);
	ROOT_VECTOR=NULL;
	segparser_scan_string(s);
	yyparse();// return std::auto_ptr<std::vector<ChromStartEnd> >();
	//segparserlex_destroy();

	return std::auto_ptr<std::vector<ChromStartEnd> >(ROOT_VECTOR);
	}
#ifdef TEST_THIS_CODE
int main(int argc,char** argv)
	{
	std::auto_ptr<std::vector<ChromStartEnd> > p= parseSegments(argv[1]);
	if(p.get()==0)
		{
		cerr << "NULL"  << endl;
		return -1;
		}
	for(int i=0;i< p->size();++i)
		{
		cerr << p->at(i) << endl;
		}
	return 0;
	}
#endif
