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
#include <mysql.h>
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"

using namespace std;



class MysqlQuery
    {
    public:
	 MYSQL* mysql;
	 int limit;
	 int select_type;
	 Tokenizer tokenizer;
	 string query;
	 MysqlQuery():
		 mysql(NULL),
		 limit(-1)
	     {
	     if((mysql=::mysql_init(NULL))==NULL) THROW("Cannot init mysql");
	     }



	 ~MysqlQuery()
	     {
	     ::mysql_close(mysql);
	     }
	 string escape(string s)
	 	 {
		 char* to=new char[s.size()*2+1];
		 mysql_real_escape_string(mysql,to,s.c_str(),s.size());
		 string esc(to);
		 delete[] to;
		 return esc;
	 	 }

	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     size_t nLine=0;
	     vector<string> tokens;
	     ostringstream os;
	     int ncols=-1;
	     vector<string> headerLines;

	     while(getline(in,line,'\n'))
			 {
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
			char* p=(char*)query.c_str();
			char* prev=p;
			while((p=strchr(prev,'$'))!=NULL)
				{
				if(!isdigit(*(p+1)))
					{

					os.write(prev,prev-p);
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


			 bool found=false;
			 string query(os.str());
			 MYSQL_ROW row;
			 if(mysql_real_query( mysql, query.c_str(),query.size())!=0)
				 {
				 cerr << "Failure for "<< query << "\n";
				 cerr << line << endl;
				 cerr << mysql_error(mysql)<< endl;
				 continue;
				 }
			 MYSQL_RES* res=mysql_use_result( mysql );
			 if(ncols==-1) ncols=mysql_field_count(mysql);
			 if(!headerLines.empty())
			 	 {
				 MYSQL_FIELD* fields;
				 for(size_t i=0;i< headerLines.size();++i)
				 	 {
					 cout << headerLines[i];
					 while((fields=mysql_fetch_field(res))!=NULL)
					 	 {
						 cout << tokenizer.delim;
						 cout << (fields->name==NULL?".":fields->name);
					 	 }
					 cout << endl;
				 	 }
				 headerLines.clear();
			 	 }
			 int nOut=0;
			 while(( row = mysql_fetch_row( res ))!=NULL )
				 {
				 if(nOut+1> limit && limit!=-1) break;
				 cout << line ;
				 for(int i=0;i< ncols;++i)
					 {
					 cout << tokenizer.delim << row[i];
					 }
				 cout << endl;
				 found=true;

				 ++nOut;
				 }
			 ::mysql_free_result( res );

			 if(!found)
				 {
				 cout << line;
				 for(int i=0;i< ncols;++i)
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
				 for(int j=0;j< ncols;++j)
					 {
					 cout << tokenizer.delim;
					 cout << ".";
					 }
				 cout << endl;
				 }
			 headerLines.clear();
			 }
	     }

    };

int main(int argc,char** argv)
    {
    MysqlQuery app;
    int optind=1;
    string host("genome-mysql.cse.ucsc.edu");
    string username("genome");
    string password;
    string database("hg19");
    vector<string> custom_fields;

    int port=0;
    while(optind < argc)
	    {
	    if(std::strcmp(argv[optind],"-h")==0)
		    {
		    cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
		    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    cerr << "Options:\n";
		    cerr << "  -d (char) delimiter default:tab\n";
		    cerr << "  --host mysql host ( " << host << ")\n";
		    cerr << "  --user mysql user ( " << username << ")\n";
		    cerr << "  --password mysql password ( " << password << ")\n";
		    cerr << "  --database mysql db ( " << database << ")\n";
		    cerr << "  --port (int) mysql port ( default)\n";
		    cerr << "  -e or -q (SQL query)\n";
		    cerr << "  -L (int) limit number or rows returned\n";
		    cerr << "(stdin|files)\n\n";
		    exit(EXIT_FAILURE);
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
				return EXIT_FAILURE;
				}
			}
	    else if(std::strcmp(argv[optind],"--host")==0 && optind+1<argc)
			{
			host.assign(argv[++optind]);
			}
	    else if(std::strcmp(argv[optind],"--user")==0 && optind+1<argc)
			{
			username.assign(argv[++optind]);
			}
	    else if(std::strcmp(argv[optind],"--password")==0 && optind+1<argc)
			{
			password.assign(argv[++optind]);
			}
	    else if(std::strcmp(argv[optind],"--port")==0 && optind+1<argc)
			{
			port=atoi(argv[++optind]);
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
			exit(EXIT_FAILURE);
			}
	    else
			{
			break;
			}
	    ++optind;
	       }



    if(mysql_real_connect(
	    app. mysql,
	    host.c_str(),
	    username.c_str(),
	    password.c_str(),
	    database.c_str(), port,NULL, 0 )==NULL)
		{
		cerr << "mysql_real_connect failed." << endl;
		return EXIT_FAILURE;
		}
    if(app.query.empty())
    	{
    	cerr << "undefined query." << endl;
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
			char* filename=argv[optind++];
			igzstreambuf buf(filename);
			istream in(&buf);
			app.run(in);
			buf.close();
			}
	    }
    return EXIT_SUCCESS;
    }
