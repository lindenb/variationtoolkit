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
#include <sqlite3.h>
#include "application.h"
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "cescape.h"

using namespace std;



class SqliteQuery:public AbstractApplication
    {
    public:
	 sqlite3* connection;
	 int limit;
	 string query;
	 SqliteQuery():
	     connection(NULL),
	     limit(-1)
	     {

	     }



	 ~SqliteQuery()
	     {
	     if(connection!=NULL)
		 {
		 ::sqlite3_close(connection);
		 connection=NULL;
		 }
	     }

	 string escape(string s)
	 	 {
		 CEscape esc(s);
		 return  esc.str();
	 	 }

	 struct Shuttle
	     {
	     SqliteQuery* owner;
	     std::string* line;
	     int ncols;
	     vector<string>* headerLines;
	     int nOut;
	     bool found;
	     };

	 static  int callback(void* ptr,int n_columns,char** columns,char**labels)
	     {
	     if(AbstractApplication::stopping()) return 0;
	     Shuttle* shuttle=(Shuttle*)ptr;
	     if(shuttle->ncols==-1) shuttle->ncols=n_columns;
	     if(!shuttle->headerLines->empty())
		 {
		 for(size_t i=0;i< shuttle->headerLines->size();++i)
		     {
		     cout << shuttle->headerLines->at(i);
		     for(int j=0;j< n_columns;++j)
			 {
			 cout << shuttle->owner->tokenizer.delim;
			 cout << (labels[i]==NULL?".":labels[i]);
			 }
		     cout << endl;
		     }
		 shuttle->headerLines->clear();
		 }

	     if(shuttle->nOut+1> shuttle->owner->limit && shuttle->owner->limit!=-1) return 0;
	     cout << *(shuttle->line) ;
	     for(int i=0;i< n_columns;++i)
		 {
		 cout <<  shuttle->owner->tokenizer.delim << columns[i];
		 }
	     cout << endl;
	     shuttle->found=true;

	     shuttle->nOut++;

	     return 0;
	     }

	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     size_t nLine=0;
	     vector<string> tokens;
	     ostringstream os;

	     vector<string> headerLines;
	     Shuttle shuttle;
	     shuttle.owner=this;
	     shuttle.line=&line;
	     shuttle.ncols=-1;
	     shuttle.headerLines=&headerLines;
	     shuttle.found=true;
	     shuttle.nOut=0;

	     while(getline(in,line,'\n'))
		 {
		 if(AbstractApplication::stopping()) break;
		 ++nLine;
		 tokenizer.split(line,tokens);
		 if(line.empty()) continue;
		 if(line[0]=='#')
		     {
		     if(line.size()>1 && line[1]=='#')
			 {
			 cout << line << endl;
			 continue;
			 }
		     headerLines.push_back(line);
		     continue;
		     }


		 ostringstream os;
		 bool ok=true;
		 char* p=(char*)this->query.c_str();
		 char* prev=p;

		 while((p=strchr(prev,'$'))!=NULL)
		     {
		     if(!isdigit(*(p+1)))
			 {
			 os.write(prev,p-prev);
			 prev=p+1;
			 continue;
			 }

		     os.write(prev,p-prev);
		     ++p;
		     char *p2;
		     int col=(int)strtol(p,&p2,10);
		     if(col<1)
			 {
			 THROW("Bad query near \""<< p<< "\"");
			 }
		     col--;//0-based
		     if(col>=(int)tokens.size())
			 {
			 ok=false;
			 cerr << "Index out of bound for "<< query << " and "<< line << endl;
			 break;
			 }
		     os << escape(tokens[col]);
		     prev=p2;

		     }
		 os << prev;

		 if(!ok) continue;



		 string sql(os.str());

		 shuttle.nOut=0;
		 shuttle.found=false;
		 char *errmsg=NULL;
		 int ret;
		 if((ret=::sqlite3_exec(connection,
		     sql.c_str(),
	             SqliteQuery::callback,
		     &shuttle,
		     &errmsg))!=0)
		     {
		     cerr << "Failure for "<< sql;
		     if(errmsg!=NULL) cerr << " "<< errmsg << " ";
		     cerr << endl;
		     }
		 if(errmsg!=NULL) sqlite3_free(errmsg);
		 if(ret!=0) continue;


		 if(!shuttle.found)
		     {
		     cout << line;
		     for(int i=0;i< shuttle.ncols;++i)
			 {
			 cout << tokenizer.delim << ".";
			 }
		     cout << endl;
		     }
		 }

	     if(!headerLines.empty())
		 {
		 for(size_t i=0;i< headerLines.size();++i)
		     {
		     cout << headerLines[i];
		     for(int j=0;j< shuttle.ncols;++j)
			 {
			 cout << tokenizer.delim;
			 cout << ".";
			 }
		     cout << endl;
		     }
		 headerLines.clear();
		 }
	     }
	void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -d (char) delimiter default:tab\n";
	    out << "  -f <sqlite-db>\n";
	    out << "  -e or -q (SQL query)\n";
	    out << "  -L (int) limit number or rows returned\n";
	    out << "(stdin|files)\n\n";
	    }

    };

int main(int argc,char** argv)
    {
    SqliteQuery app;
    int optind=1;
    const char* sqlitedb=NULL;

    while(optind < argc)
	{
	if(std::strcmp(argv[optind],"-h")==0)
	    {
	    app.usage(cout,argc,argv);
	    return EXIT_SUCCESS;
	    }
	else if((std::strcmp(argv[optind],"-e")==0  || std::strcmp(argv[optind],"-q")==0 ) && optind+1<argc)
	    {
	    app.query=(argv[++optind]);
	    }
	else if(std::strcmp(argv[optind],"-L")==0 && optind+1<argc)
	    {
	    char* p2;
	    app.limit=(int)strtol(argv[++optind],&p2,10);
	    if(app.limit<1 || *p2!=0)
		{
		cerr << "Bad limit\n";
		app.usage(cerr,argc,argv);
		return EXIT_FAILURE;
		}
	    }
	else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
	    {
	    sqlitedb=(argv[++optind]);
	    }
	else if(std::strcmp(argv[optind],"-d")==0 && optind+1< argc)
	    {
	    char* p=argv[++optind];
	    if(strlen(p)!=1)
		{
		cerr << "Bad delimiter \""<< p << "\"\n";
		exit(EXIT_FAILURE);
		}
	    app.tokenizer.delim=p[0];
	    }
	else if(argv[optind][0]=='-')
	    {
	    cerr<< "unknown option '" << argv[optind] << "'." << endl;
	    app.usage(cerr,argc,argv);
	    return(EXIT_FAILURE);
	    }
	else
	    {
	    break;
	    }
	++optind;
	}

    if(sqlitedb==NULL)
	{
	cerr<< "undefined sqlite db." << endl;
	app.usage(cerr,argc,argv);
	return(EXIT_FAILURE);
	}

    if(app.query.empty())
   	{
   	cerr << "undefined query." << endl;
   	return EXIT_FAILURE;
   	}

    if(::sqlite3_open_v2(sqlitedb,&app.connection,SQLITE_OPEN_READONLY,NULL)!=SQLITE_OK)
   	{
   	cerr << "Cannot open sqlite file "<< sqlitedb <<".\n";
   	return EXIT_FAILURE;
   	}





    if(optind==argc)
	{
	igzstreambuf buf;
	istream in(&buf);
	app.run(in);
	}
    else
	{
	while(optind< argc)
	    {
	    if(AbstractApplication::stopping()) break;
	    char* filename=argv[optind++];
	    igzstreambuf buf(filename);
	    istream in(&buf);
	    app.run(in);
	    buf.close();
	    }
	}
    return EXIT_SUCCESS;
    }
