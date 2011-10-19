%{
#include <set>
#include <string>
#include <vector>
#include <limits>
#include <iostream>
#include <memory>
#include <stdint.h>
#include "throw.h"
#define yylex  selectsetlex
using namespace std;

class Node
	{
	protected:
		Node(){}
	public:
		virtual ~Node(){}
		virtual bool eval(const set<string>& s)=0;
		static Node* ROOT;	
	};

class YNode:public Node
	{
	public:
		Node* left;
		Node* right;
		YNode(Node* n1,Node* n2):left(n1),right(n2) {}
		virtual ~YNode()
			{
			delete left;
			delete right;
			}
		virtual bool eval(const set<string>& s)=0;

	};

class OrNode:public YNode
	{
	public:
		OrNode(Node* n1,Node* n2):YNode(n1,n2) {}
		virtual ~OrNode() {}
		virtual bool eval(const set<string>& s)
			{
			return left->eval(s) || right->eval(s);  
			}
	};
	
class AndNode:public YNode
	{
	public:
		AndNode(Node* n1,Node* n2):YNode(n1,n2) {}
		virtual ~AndNode() {}
		virtual bool eval(const set<string>& s)
			{
			return left->eval(s) && right->eval(s);  
			}	
	};

class NotNode:public Node
	{
	public:
		Node* node;
		NotNode(Node* n):node(n) {}
		virtual ~NotNode() {}
		virtual bool eval(const set<string>& s)
			{
			return !(node->eval(s));  
			}
	};
	
class StringNode:public Node
	{
	public:
		std::string* s;
		StringNode(std::string* s):s(s)
			{
			}
		virtual ~StringNode()
			{
			delete s;
			}
		virtual bool eval(const set<string>& data)
			{
			return data.find(*s)!=data.end();
			}
	};

Node* Node::ROOT=NULL;

extern int selectsetlex(); 
#define yylex selectsetlex

void yyerror (char *s)
 	{
 	THROW("Parsing error :\""<< (const char*)s<< "\"");
 	}

%}

%error-verbose

%union	{
	std::string* s;
	Node* n;
	}
	
%token <s> TEXT
%type <n> node orExpr andExpr termNode
%token LEX_AND LEX_OR LEX_NOT OPAR CPAR
%left LEX_NOT
%start input
%% 

input: node { Node::ROOT=$1;}
	;
	
node	: orExpr { $$=$1;}
	| LEX_NOT node { $$=new NotNode($2);} ;

orExpr: andExpr { $$=$1;}
	| orExpr LEX_OR andExpr { $$=new OrNode($1,$3);}
	;
andExpr	: termNode { $$=$1;}
	| andExpr LEX_AND termNode  { $$=new AndNode($1,$3);}
	;

termNode: OPAR node CPAR { $$=$2;}
	| TEXT { $$=new StringNode($1);}

	;
%%


void* selectSetParse(const char* s)
	{
	extern int selectset_scan_string(const char *);
	Node::ROOT=NULL;
	selectset_scan_string(s);
	yyparse();
	void* ptr=Node::ROOT;
	Node::ROOT=NULL;
	return ptr;
	}
	
void selectSetFree(void* ptr)
	{
	Node* n=(Node*)ptr;
	if(n==NULL) return;
	delete n;
	}

bool selectSetEval(void* ptr,const set<string>& s)
	{
	Node* n=(Node*)ptr;
	if(n==NULL) return true;
	return n->eval(s);
	}