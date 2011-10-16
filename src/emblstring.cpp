/*
 * mysqlucsc.cpp
 *
 *  Created on: Oct 2, 2011
 *      Author: lindenb
 */
#include <cstdlib>
#include <vector>
#include <map>
#include <set>
#include <cerrno>
#include <fstream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <iostream>
#include <zlib.h>
#include <sstream>
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include "netstreambuf.h"
#include "application.h"
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "httpescape.h"

using namespace std;

/**
 * AbstractEmblSTRING
 */
class AbstractEmblSTRING:public AbstractApplication
    {
    public:
  	 int taxon;
  	 int identifierColumn;
  	 AbstractEmblSTRING()
  	     {
  	     identifierColumn=-1;
  	     taxon=9606;
  	     }
  	 virtual ~AbstractEmblSTRING()
  	     {
  	     }

  	virtual int numHeaders()=0;
  	virtual void printHeader()=0;
  	virtual bool process(const string& line,const string& identier)=0;
  	virtual void run(std::istream& in)
	     {
	     string line;
	     vector<string> tokens;

	     while(std::getline(in,line,'\n'))
		 {
		 if(stopping()) break;
		 if(line.empty()) continue;
		 if(line[0]=='#')
		     {
		     if(line.size()>1 && line[1]=='#')
			 {
			 cout << line << endl;
			 continue;
			 }
		     cout << line;
		     printHeader();
		     cout << endl;
		     continue;
		     }
		 tokenizer.split(line,tokens);
		 if(this->identifierColumn>=(int)tokens.size())
		     {
		     cerr << "Column out of bound for "<< line << endl;
		     continue;
		     }
		 bool found = process(line,tokens[identifierColumn]);

		 if(!found)
		     {
		     cout << line;
		     for(int i=0;i< numHeaders();++i)
			 {
			 cout << tokenizer.delim << ".";
			 }
		     cout << endl;
		     }
		 }
	     }

  	 int main(int argc,char** argv)
  	     {
  	     int optind=1;
  	     int n_optind;
  	     while(optind < argc)
  		 {
  		 if(std::strcmp(argv[optind],"-h")==0)
  		     {
  		     this->usage(cout,argc,argv);
  		     return EXIT_SUCCESS;
  		     }
  		 else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
  		     {
  		     char* p=argv[++optind];
  		     if(strlen(p)!=1)
  			 {
  			 cerr << "Bad delimiter \""<< p << "\"\n";
  			 exit(EXIT_FAILURE);
  			 }
  		     this->tokenizer.delim=p[0];
  		     }
  		 else if(std::strcmp(argv[optind],"-c")==0 && optind+1< argc)
		     {
		     char* p=argv[++optind];
		     char* p2=NULL;
		     identifierColumn=(int)strtol(p,&p2,10);
		     if(identifierColumn<1 || *p2!=0)
			 {
			 cerr << "Bad column identifier" << endl;
			 return EXIT_FAILURE;
			 }
		     identifierColumn--;
		     }
  		 else if(std::strcmp(argv[optind],"-t")==0 && optind+1< argc)
		     {
		     char* p=argv[++optind];
		     char* p2=NULL;
		     taxon=(int)strtol(p,&p2,10);
		     if(taxon<1 || *p2!=0)
			 {
			 cerr << "Bad taxon identifier" << endl;
			 return EXIT_FAILURE;
			 }
		     }
  		 else if((n_optind=argument(optind,argc,argv))!=-1)
  		     {
  		     if(n_optind==-2) return EXIT_FAILURE;
  		     optind=argc;
  		     }
  		 else if(argv[optind][0]=='-')
  		     {
  		     cerr<< "unknown option '" << argv[optind] << "'." << endl;
  		     this->usage(cerr,argc,argv);
  		     return(EXIT_FAILURE);
  		     }
  		 else
  		     {
  		     break;
  		     }
  		 ++optind;
  		 }

  	     if(identifierColumn==-1)
  		 {
  		 cerr << "Undefined column for identifier." << endl;
  		 return EXIT_FAILURE;
  		 }

  	     if(optind==argc)
  		 {
  		 igzstreambuf buf;
  		 istream in(&buf);
  		 this->run(in);
  		 }
  	     else
  		 {
  		 while(optind< argc)
  		     {
  		     if(stopping()) break;
  		     char* filename=argv[optind++];
  		     igzstreambuf buf(filename);
  		     istream in(&buf);
  		     this->run(in);
  		     buf.close();
  		     }
  		 }
  	     return EXIT_SUCCESS;
  	     }
    };

class EmblStringResolve:public AbstractEmblSTRING
    {
    public:
	 EmblStringResolve()
	     {
	     }

	 virtual ~EmblStringResolve()
	     {
	     }
	 virtual int numHeaders()
	     {
	     return 3;
	     }
	 virtual void printHeader()
	     {
	     cout	<< tokenizer.delim << "stringId"
			<< tokenizer.delim << "preferredName"
			<< tokenizer.delim << "annotation"
			;
	     }

	 virtual bool process(const string& line,const string& identifier)
	     {
	     bool found=false;
	     Tokenizer tab;
	     int nLine=0;
	     vector<string> tokens;
	     string line2;
	     ostringstream os;
	     os << "http://string-db.org/api/tsv/resolve?identifier="
		     << httpEscape(identifier) <<"&species="<< taxon;
	     netstreambuf buff;
	     buff.open(os.str().c_str());
	     istream in(&buff);
	     while(getline(in,line2,'\n'))
		 {
		 if(stopping()) break;
		 ++nLine;
		 tab.split(line2,tokens);
		 if(nLine==1)
		     {
		     if(!tokens[0].compare("stringId")==0)
			 {
			 return false;
			 }
		     continue;
		     }
		 if(tokens.size()!=5) THROW("Format of EMBLhas changed."<< line);
		 cout << line << tokenizer.delim <<
			 tokens[0] << tokenizer.delim <<
			 tokens[3] << tokenizer.delim <<
			 tokens[4] << endl;
		 found=true;
		 }
	     buff.close();
	     return found;
	     }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -d (char) delimiter default:tab\n";
	    out << "  -c column identifier\n";
	    out << "  -t (int) taxon id\n";
	    out << "(stdin|files)\n\n";
	    }
    };


class EmblStringInteractionList:public AbstractEmblSTRING
    {
    public:
	EmblStringInteractionList()
	     {
	     }

	 virtual ~EmblStringInteractionList()
	     {
	     }
	 virtual int numHeaders()
	     {
	     return 15;
	     }
	 virtual void printHeader()
	     {

	     cout	<< tokenizer.delim << "interactorA"
			<< tokenizer.delim << "interactorB"
			<< tokenizer.delim << "labelA"
			<< tokenizer.delim << "labelB"
			<< tokenizer.delim << "aliasesA"
			<< tokenizer.delim << "aliasesB"
			<< tokenizer.delim << "method"
			<< tokenizer.delim << "firstAuthor"
			<< tokenizer.delim << "publication"
			<< tokenizer.delim << "taxonA"
			<< tokenizer.delim << "taxonB"
			<< tokenizer.delim << "types"
			<< tokenizer.delim << "sources"
			<< tokenizer.delim << "interaction.ids"
			<< tokenizer.delim << "score"
			;
	     }

	 virtual bool process(const string& line,const string& identifier)
	     {
	     bool found=false;
	     Tokenizer tab;
	     vector<string> tokens;
	     string line2;
	     ostringstream os;
	     os << "http://string-db.org/api/psi-mi-tab/interactionsList?identifiers="
		     << httpEscape(identifier);

	     netstreambuf buff;
	     buff.open(os.str().c_str());
	     istream in(&buff);
	     while(getline(in,line2,'\n'))
		 {
		 if(stopping()) break;
		 if(line2.empty()) continue;
		 tab.split(line2,tokens);
		 if((int)tokens.size()!=numHeaders()) THROW("Format of EMBLhas changed."<< line << " :"<< tokens.size());
		 cout << line;
		 for(size_t i=0;i< tokens.size();++i)
		     {
		     cout << tokenizer.delim <<  tokens[i];
		     }
		 cout << endl;
		 found=true;
		 }
	     buff.close();
	     return found;
	     }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -d (char) delimiter default:tab\n";
	    out << "  -c column identifier\n";
	    //out << "  -t (int) taxon id\n";
	    out << "(stdin|files)\n\n";
	    }
    };

class EmblStringInteractor:public AbstractEmblSTRING
    {
    public:
	EmblStringInteractor()
	     {
	     }

	 virtual ~EmblStringInteractor()
	     {
	     }
	 virtual int numHeaders()
	     {
	     return 1;
	     }
	 virtual void printHeader()
	     {
	     cout << tokenizer.delim << "interactor";
	     }

	 virtual bool process(const string& line,const string& identifier)
	     {
	     bool found=false;
	     string line2;
	     ostringstream os;
	     os << "http://string-db.org/api/tsv/interactors?identifier="
		     << httpEscape(identifier);

	     netstreambuf buff;
	     buff.open(os.str().c_str());
	     istream in(&buff);
	     while(getline(in,line2,'\n'))
		 {
		 if(stopping()) break;
		 if(line2.empty() || line2.compare("itemId")==0) continue;
		 cout << line << tokenizer.delim <<  line2 << endl;
		 found=true;
		 }
	     buff.close();
	     return found;
	     }

	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -d (char) delimiter default:tab\n";
	    out << "  -c column identifier\n";
	    //out << "  -t (int) taxon id\n";
	    out << "(stdin|files)\n\n";
	    }
    };



int main(int argc,char** argv)
    {
#ifdef EMBL_RESOLVE
    EmblStringResolve app;
#elif EMBL_INTERACTIONS
    EmblStringInteractionList app;
#elif EMBL_INTERACTOR
    EmblStringInteractor app;
#else
#error "Undefined program"
#endif
    return app.main(argc,argv);
    }
