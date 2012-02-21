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
#define NOWHERE
#include "where.h"
#include "throw.h"
#include "zstreambuf.h"
#include "tokenizer.h"
#include "bin.h"
#include "where.h"
#include "mysqlapplication.h"
#include "numeric_cast.h"

using namespace std;



class VcfId:public MysqlApplication
    {
    public:
	 std::string table;
	 bool limitOne;
	 VcfId():
		 table("snp135"),
		 limitOne(false)
	     {
	     }


	 virtual ~VcfId()
	     {
	     }



	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     size_t nLine=0;
	     vector<string> tokens;
	     ostringstream os;
	     os << "select name from  "<< this->table << " where chrom=\"" ;
	     const string base_query(os.str());
	     WHERE("");
	     while(getline(in,line,'\n'))
			 {
			 ++nLine;
			 WHERE(nLine);
			 if(line.empty())
				{
			     WHERE("");
				 continue;
				}
			 if(line[0]=='#')
			     {
			     if(line.compare(0,6,"#CHROM")==0)
				 {
				 cout << "##ID annotated with "<< table << endl;
				 }
			     cout << line << endl;
			     continue;
			     }
			 this->tokenizer.split(line,tokens);
			 if(tokens.size()<3)
			     {
			     cerr << "Bad VCF line in "<< line << endl;
			     continue;
			     }
			 if(!(tokens[2].compare(".")==0 || tokens[2].empty()))
			     {
			     cout << line << endl;
			     continue;
			     }
			 WHERE(line);

			 int POS;
			 if(!numeric_cast<int>(tokens[1].c_str(),&POS) || POS<1)
				 {
				 cerr << "Bad POS \"" << tokens[1] << " in "<< line << endl;
				 continue;
				 }
			 WHERE("");
			 POS--;//switch to 0 based

			 ostringstream sql;
			 sql << base_query;
			 sql << tokens[0] << "\" and "
				 << " chromStart <= " << POS
				 << " AND  " << POS << " < chromEnd"
				" AND bin in ("
				;

			 binList.clear();
			 WHERE(POS);
			 UcscBin::bins(POS,POS+1,binList);
			 WHERE("");
			 for(size_t i=0;i< binList.size();++i)
				 {
				 if(i>0) sql << ",";
				 sql << binList[i];
				 }
			sql << ")";
			if(limitOne)
			    {
			    sql << " LIMIT 1";
			    }
			WHERE("");

			 set<string> rsNames;
			 string query(sql.str());
			 MYSQL_ROW row;
			 WHERE(query);
			 if(::mysql_real_query( mysql, query.c_str(),query.size())!=0)
				 {
				 cerr << "Failure for "<< query << "\n";
				 cerr << mysql_error(mysql)<< endl;
				 continue;
				 }
			 MYSQL_RES* res=mysql_use_result( mysql );
			 WHERE("");
			 while(( row = mysql_fetch_row( res ))!=NULL )
				 {
			     	 rsNames.insert(row[0]);
				 if(limitOne) break;
				 }
			 ::mysql_free_result( res );
			 WHERE("");
			for(size_t i=0;i< tokens.size();++i)
				 {
				 if(i>0) cout << "\t";
				 if(i==2)
				     {
				     if(rsNames.empty())
					 {
					 cout << ".";
					 }
				     else
					 {
					 for(set<string>::iterator r=rsNames.begin();r!=rsNames.end();++r)
					     {
					     if(r!=rsNames.begin()) cout << ",";
					     cout << (*r);
					     }
					 }
				     continue;
				     }
				 cout << tokens[i];
				 }
			 WHERE("");
			cout << endl;
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
			    cerr << argv[0] << "Pierre Lindenbaum PHD. 2012.\n";
			    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
			    cerr << "Usage:\n\t"<< argv[0]<< " [options] (vcf|vcf.gz|stdin)*\n\n";
			    cerr << "Options:\n";
			    cerr << "  --table or -T (string) default:" << table <<"\n";
			    cerr << "  -f  (limit to one hit)\n";
			    cerr << "(stdin|files)\n\n";
			    this->printConnectOptions(cerr);
			    exit(EXIT_FAILURE);
			    }
		    else if((n_optind=this->argument(optind,argc,argv))!=-1)
			{
			if(optind==-2) return EXIT_FAILURE;
			optind=n_optind;
			}
		    else if((std::strcmp(argv[optind],"--table")==0 || std::strcmp(argv[optind],"-T")==0) && optind+1<argc)
			{
			this->table.assign(argv[++optind]);
			}
		    else if((std::strcmp(argv[optind],"-f")==0) && optind+1<argc)
			{
			this->limitOne=true;
			}
		    else if(argv[optind][0]=='-')
			{
			cerr<<"unknown option '"<<argv[optind]<<"'"<< endl;
			exit(EXIT_FAILURE);
			}
		    else
			{
			break;
			}
		++optind;
		 }

	    this->connect();
	    if(optind==argc)
		    {
		 WHERE("stdin");
		    igzstreambuf buf;
		    istream in(&buf);
		    this->run(in);
		    }
	    else
		    {
		    while(optind< argc)
				{
				char* filename=argv[optind++];
				WHERE(filename);
				igzstreambuf buf(filename);
				istream in(&buf);
				this->run(in);
				buf.close();
				}
		    }
	    return EXIT_SUCCESS;
	    }
    };

int main(int argc,char** argv)
    {
    VcfId app;
    return app.main(argc,argv);
    }
