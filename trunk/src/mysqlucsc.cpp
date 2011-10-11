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

using namespace std;

#define WHERE(a) cerr << __FILE__ << ":" << __LINE__ << ":" << a << endl

#define THROW(a) do{ostringstream _os;\
	_os << __FILE__ << ":"<< __LINE__ << ":"<< a << endl;\
	throw runtime_error(_os.str());\
	}while(0)

static void binsInRange(
        int chromStart,
        int chromEnd,
        int binId,
        int level,
        int binRowStart,
        int rowIndex,
        int binRowCount,
        int genomicPos,
        int genomicLength,
        vector<int>& binList
        )
        {
	binList.push_back(binId);
        if(level<4)
		{
		int i;
		int childLength=genomicLength/8;
		int childBinRowCount=binRowCount*8;
		int childRowBinStart=binRowStart+binRowCount;
		int firstChildIndex=rowIndex*8;
		int firstChildBin=childRowBinStart+firstChildIndex;
		for(i=0;i< 8;++i)
		        {
		        int childStart=genomicPos+i*childLength;

		        if( chromStart>(childStart+childLength) ||
		                chromEnd<childStart )
		                {
		                continue;
		                }
		        binsInRange(
		                chromStart,
		                chromEnd,
		                firstChildBin+i,
		                level+1,
		                childRowBinStart,
		                firstChildIndex+i,
		                childBinRowCount,
		                childStart,
		                childLength,
				binList
		                );
		        }
		}
        }



static void bins(int chromStart,int chromEnd,vector<int>& binList)
	{
	int genomicLength=536870912;
	binList.clear();
	binsInRange(chromStart,chromEnd,0,0,0,0,1,0,genomicLength,binList);
	}

enum	{
    select_any=0,
    select_user_IN_sql=1,
    select_user_OUT_sql=2
};

class Table
    {
    public:
	bool hasBin;
	string name;
	string chrom;
	string chromStart;
	string chromEnd;
	vector<string> columns;
    };

class MysqlUcsc
    {
    public:
	 MYSQL* mysql;
	 Table* table;
	 char delim;
	 bool first_line_header;
	 int chromcol;
	 int startcol;
	 int endcol;
	 int limit;
	 bool data_are_plus1_based;
	 int select_type;
	 MysqlUcsc():
		 mysql(NULL),
		 table(NULL),
		 delim('\t'),
		 first_line_header(true),
		 limit(-1),
		 data_are_plus1_based(false),
		 select_type(select_any)
	     {
	     chromcol=-1;
	     startcol=-1;
	     endcol=-1;
	     if((mysql=::mysql_init(NULL))==NULL) THROW("Cannot init mysql");
	     }



	 ~MysqlUcsc()
	     {
	     if(table!=NULL) delete table;
	     ::mysql_close(mysql);
	     }

	 void split(const string& line,vector<string>& tokens)
		{
		size_t prev=0;
		size_t i=0;
		tokens.clear();
		while(i<=line.size())
		    {
		    if(i==line.size() || line[i]==delim)
			{
			tokens.push_back(line.substr(prev,i-prev));
			if(i==line.size()) break;
			prev=i+1;
			}
		    ++i;
		    }
		}

	 Table* schema(const char* tableName)
	     {
	     Table* table=new Table;
	     table->hasBin=false;
	     table->name.assign(tableName);
	     MYSQL_ROW row;
	     ostringstream os;
	     os << "desc "<< tableName;
	     string query=os.str();
	     mysql_real_query( mysql, query.c_str(),query.size());
	     MYSQL_RES*	res=mysql_use_result( mysql );
	     //int num_fields = mysql_num_fields(res);
	     while(( row = mysql_fetch_row( res ))!=NULL )
		     {
		     unsigned long *lengths= mysql_fetch_lengths(res);
		     if(lengths==NULL) THROW("cannot fetch length");
		     string colName(row[0],lengths[0]);
		     table->columns.push_back(colName);
		     if(colName.compare("bin")==0)
			 {
			 table->hasBin=true;
			 }
		     else if(colName.compare("chrom")==0)
			 {
			 table->chrom=colName;
			 }
		     else if(colName.compare("chromStart")==0)
			 {
			 table->chromStart=colName;
			 }
		     else if(colName.compare("chromEnd")==0)
			 {
			 table->chromEnd=colName;
			 }
		     }
	     mysql_free_result( res );
	     if(table->chrom.empty())
		 {
		 cerr << "Cannot find chrom column in "<< tableName << endl;
		 delete table;
		 return 0;
		 }
	     if(table->chromStart.empty())
		 {
		 cerr << "Cannot find chromStart column in "<< tableName << endl;
		 delete table;
		 return 0;
		 }
	     if(table->chromEnd.empty())
		 {
		 cerr << "Cannot find chromEnd column in "<< tableName << endl;
		 delete table;
		 return 0;
		 }
	     return table;
	     }

	 void run(std::istream& in)
	     {
	     string line;
	     vector<int> binList;
	     size_t nLine=0;
	     vector<string> tokens;
	     ostringstream os;
	     os << "select ";
	     for(size_t i=0;i<  table->columns.size();++i)
		 {
		 if(i>0) os << ",";
		 os << table->columns[i];
		 }
	     os << " from "<< table->name << " where chrom=\"" ;
	     const string base_query(os.str());

	     while(getline(in,line,'\n'))
		 {
		 ++nLine;
		 split(line,tokens);
		 if(nLine==1 && first_line_header)
		     {
		     for(size_t i=0;i< tokens.size();++i)
			 {
			 if(i>0) cout << delim;
			 cout << tokens[i];
			 }
		     for(size_t i=0;i< table->columns.size();++i)
			 {
			 cout << delim<< table->columns[i];
			 }
		     cout <<endl;
		     continue;
		     }
		 if((size_t)chromcol>=tokens.size())
		     {
		     cerr << "Chrom column: index out of range in "<< line << endl;
		     continue;
		     }
		 if((size_t)startcol>=tokens.size())
		     {
		     cerr << "START column: index out of range in "<< line << endl;
		     continue;
		     }
		 if((size_t)endcol>=tokens.size())
		     {
		     cerr << "END column: index out of range in "<< line << endl;
		     continue;
		     }

		 char* p2;
		 int chromStart=strtol(tokens[startcol].c_str(),&p2,10);
		 if(*p2!=0)
		     {
		     cerr << "Bad chromStart in "<< line << endl;
		     continue;
		     }
		 int chromEnd=strtol(tokens[endcol].c_str(),&p2,10);
		 if(*p2!=0)
		     {
		     cerr << "Bad chromStart in "<< line << endl;
		     continue;
		     }
		 if(data_are_plus1_based)
		     {
		     chromStart--;
		     chromEnd--;
		     }

		 ostringstream sql;
		 sql << base_query;
		 sql << tokens[chromcol] << "\" and ";
		 switch(select_type)
		     {
		     case select_user_IN_sql:
			 {
			 sql << table->chromStart << " <= " << tokens[startcol]
			    << " and "<< table->chromEnd << " >= " << tokens[endcol];

			 break;
			 }
		     case select_user_OUT_sql:
			 {
			 sql << table->chromStart << " >= " << tokens[startcol]
			    << " and "<< table->chromEnd << " <= " << tokens[endcol]
			     ;
			 break;
			 }
		     default:
			 {
			 sql << " NOT(" << table->chromEnd << " <= " << tokens[startcol]
			    << " or "<< table->chromStart << " > " << tokens[endcol]
			    << ")";
			 break;
			 }
		     }
		 if(table->hasBin)
		     {
		     binList.clear();
		     bins(chromStart,chromEnd,binList);
		     sql << " and bin in (";
		     for(size_t i=0;i< binList.size();++i)
			 {
			 if(i>0) sql << ",";
			 sql << binList[i];
			 }
		     sql << ")";
		     }
		  if(limit>0)
		     {
		     sql << " limit "<< limit;
		     }
		 bool found=false;
		 string query(sql.str());

		 MYSQL_ROW row;
		 if(mysql_real_query( mysql, query.c_str(),query.size())!=0)
		     {
		     cerr << "Failure for "<< query << "\n";
		     cerr << mysql_error(mysql)<< endl;
		     continue;
		     }
		 MYSQL_RES* res=mysql_use_result( mysql );
		 int ncols=mysql_field_count(mysql);
		 while(( row = mysql_fetch_row( res ))!=NULL )
			 {
			 for(size_t i=0;i< tokens.size();++i)
			     {
			     if(i>0) cout << delim;
			     cout << tokens[i];
			     }
			 for(int i=0;i< ncols;++i)
			     {
			     cout << delim << row[i];
			     }
			 cout << endl;
			 found=true;
			 }
		 ::mysql_free_result( res );

		 if(!found)
		     {
		     for(size_t i=0;i< tokens.size();++i)
			 {
			 if(i>0) cout << delim;
			 cout << tokens[i];
			 }
		     for(size_t i=0;i< table->columns.size();++i)
			 {
			 cout << delim ;
			 }
		     cout << endl;
		     }

		 }
	     }

    };

int main(int argc,char** argv)
    {
    MysqlUcsc app;
    int optind=1;
    string host("genome-mysql.cse.ucsc.edu");
    string username("genome");
    string password;
    string database("hg19");
    vector<string> custom_fields;
    char* tablename=NULL;
    int port=0;
    while(optind < argc)
	    {
	    if(std::strcmp(argv[optind],"-h")==0)
		    {
		    cerr << argv[0] << "Pierre Lindenbaum PHD. 2011.\n";
		    cerr << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
		    cerr << "Options:\n";
		    cerr << "  --delim (char) delimiter default:tab\n";
		    cerr << "  --host mysql host ( " << host << ")\n";
		    cerr << "  --user mysql user ( " << username << ")\n";
		    cerr << "  --password mysql password ( " << password << ")\n";
		    cerr << "  --database mysql db ( " << database << ")\n";
		    cerr << "  --port (int) mysql port ( default)\n";
		    cerr << "  --table or -T (string)\n";
		    cerr << "  -C (int) chromosome column (first is 1).\n";
		    cerr << "  -S (int)start column (first is 1).\n";
		    cerr << "  -E (int) end column (first is 1).\n";
		    cerr << "  -f first column is not header.\n";
		    cerr << "  -1 data are +1 based.\n";
		    cerr << "  --limit (int) limit number or rows returned\n";
		    cerr << "  --field <string> set custom field. Can be used several times\n";
		    cerr << "  --type (int) type of selection: 0 any (default), 1 user data IN mysql data,2 user data embrace mysql data.\n";
		    cerr << "(stdin|files)\n\n";
		    exit(EXIT_FAILURE);
		    }
	    else if(std::strcmp(argv[optind],"--type")==0 && optind+1<argc)
		{
		char* p2;
		app.select_type=(int)strtol(argv[++optind],&p2,10);
		if(app.select_type<0 || app.select_type>2)
		    {
		    cerr << "Bad select type\n";
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"--field")==0 && optind+1<argc)
		{
		custom_fields.push_back(argv[++optind]);
		}
	    else if(std::strcmp(argv[optind],"-1")==0 && optind+1<argc)
		{
		app.data_are_plus1_based=true;
		}
	    else if(std::strcmp(argv[optind],"--limit")==0 && optind+1<argc)
		{
		char* p2;
		app.limit=(int)strtol(argv[++optind],&p2,10);
		if(app.limit<1 || *p2!=0)
		    {
		    cerr << "Bad limit\n";
		    return EXIT_FAILURE;
		    }
		}
	    else if(std::strcmp(argv[optind],"-C")==0 && optind+1<argc)
		{
		char* p2;
		app.chromcol=(int)strtol(argv[++optind],&p2,10);
		if(app.chromcol<1 || *p2!=0)
		    {
		    cerr << "Bad CHROM column.\n";
		    return EXIT_FAILURE;
		    }
		app.chromcol--;
		}
	    else if(std::strcmp(argv[optind],"-S")==0 && optind+1<argc)
		{
		char* p2;
		app.startcol=(int)strtol(argv[++optind],&p2,10);
		if(app.startcol<1 || *p2!=0)
		    {
		    cerr << "Bad START column.\n";
		    return EXIT_FAILURE;
		    }
		app.startcol--;
		}
	    else if(std::strcmp(argv[optind],"-E")==0 && optind+1<argc)
		{
		char* p2;
		app.endcol=(int)strtol(argv[++optind],&p2,10);
		if(app.endcol<1 || *p2!=0)
		    {
		    cerr << "Bad END column.\n";
		    return EXIT_FAILURE;
		    }
		app.endcol--;
		}
	    else if(std::strcmp(argv[optind],"-f")==0 && optind+1<argc)
		{
		app.first_line_header=false;
		}
	    else if((std::strcmp(argv[optind],"--table")==0 || std::strcmp(argv[optind],"-T")==0) && optind+1<argc)
		{
		tablename=(argv[++optind]);
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
	    else if(std::strcmp(argv[optind],"--delim")==0 && optind+1< argc)
		{
		char* p=argv[++optind];
		if(strlen(p)!=1)
		    {
		    cerr << "Bad delimiter \""<< p << "\"\n";
		    exit(EXIT_FAILURE);
		    }
		app.delim=p[0];
		}
	    else if(argv[optind][0]=='-')
		{
		fprintf(stderr,"unknown option '%s'\n",argv[optind]);
		exit(EXIT_FAILURE);
		}
	    else
		{
		break;
		}
	    ++optind;
	       }

    if(tablename==NULL)
	{
	cerr << "undefined table" << endl;
	return EXIT_FAILURE;
	}
    if(app.chromcol<0)
    	{
    	cerr << "undefined CHROM col"<< endl;
    	return EXIT_FAILURE;
    	}
    if(app.startcol<0)
	{
	cerr << "undefined START col"<< endl;
	return EXIT_FAILURE;
	}
    if(app.endcol<0) app.endcol=app.startcol;
    if(mysql_real_connect(
	    app. mysql,
	    host.c_str(),
	    username.c_str(),
	    password.c_str(),
	    database.c_str(), port,NULL, 0 )==NULL)
	{
	THROW("mysql_real_connect failed.");
	}
    app.table=app.schema(tablename);
    if(!custom_fields.empty())
	{
	app.table->columns.clear();
	for(size_t i=0;i< custom_fields.size();++i)
	    {
	    app.table->columns.push_back(custom_fields[i]);
	    }
	}
    if(app.table==NULL)
	{
	cerr << "Cannot get table "<< database<<"."<< tablename << endl;
	return EXIT_FAILURE;
	}
    if(optind==argc)
	    {
	    app.run(cin);
	    }
    else
	    {
	    while(optind< argc)
		{
		char* filename=argv[optind++];
		fstream in(filename,ios::in);
		if(!in.is_open())
		    {
		    cerr << "Cannot open "<< filename << " " << strerror(errno) << endl;
		    return EXIT_FAILURE;
		    }
		app.run(in);
		in.close();
		}
	    }
    return EXIT_SUCCESS;
    }
