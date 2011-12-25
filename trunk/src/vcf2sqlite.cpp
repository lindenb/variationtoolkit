/*
 * vcf2sqlite.cpp
 *
 *  Created on: Dec 25, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include "xsqlite.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "where.h"
using namespace std;

#define COLUMN_ID "id INTEGER PRIMARY KEY AUTOINCREMENT,"
#define COLUMN_CREATED "created timestamp NOT NULL default (datetime()),"

class VcfToSqlite:public AbstractApplication
    {
    public:
	char* dataasename;
	auto_ptr<Connection> connection;
	auto_ptr<Statement> insert_vcffile;

	VcfToSqlite():dataasename(NULL)
	    {

	    }

	void close()
	    {

	    }

	void open()
	    {
	    if(dataasename==0) THROW("DB name undefined.");
	    ConnectionFactory cf;
	    cf.set_allow_create(true);
	    cf.set_read_only(false);
	    cf.set_filename(dataasename);
	    this->connection=cf.create();
	    this->connection->execute(
		   "create table if not exists META("
		    COLUMN_ID
		    COLUMN_CREATED
		   "prop TEXT NOT NULL UNIQUE, "
		   "value TEXT NOT NULL "
		   ")");
	    /*this->connection->execute(
		"INSERT OR IGNORE into META(prop,value) values(\"varkit.revision\",\"" VARKIT_REVISION "\")"
		);*/
	    this->connection->execute(
	       "create table if not exists VCFFILE("
		COLUMN_ID
		COLUMN_CREATED
	       "filename TEXT NOT NULL"
	       ")");
	    insert_vcffile = this->connection->prepare(
	       "insert into VCFFILE(filename) values (?)"
		);

	    }

	void run(const char* filename,std::istream& in)
	    {
	    insert_vcffile->bind_string(1,filename);
	    insert_vcffile->execute();
	    int64_t vcf_file_id= connection->last_insert_id();
	    cerr << "VCF id="<< vcf_file_id << endl;

	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
		{

		}
	    }


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2011.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Inserts a VCF in a sqlite3 database.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -f (file) sqlite filename. (REQUIRED).\n";
	    out << "(stdin|vcf|vcf.gz)\n\n";
	    }

	int main(int argc,char** argv)
		{
		int optind=1;

		while(optind < argc)
		    {
		    if(std::strcmp(argv[optind],"-h")==0)
			{
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else if(strcmp(argv[optind],"-f")==0 && optind+1<argc)
			{
			dataasename=argv[++optind];
			}
		    else if(argv[optind][0]=='-')
			{
			cerr << "unknown option '"<< argv[optind]<<"'\n";
			this->usage(cerr,argc,argv);
			return (EXIT_FAILURE);
			}
		    else
			{
			break;
			}
		    ++optind;
		    }
		if(dataasename==NULL)
		    {
		    cerr << "Undefined sqlite database."<< endl;
		    this->usage(cerr,argc,argv);
		    return (EXIT_FAILURE);
		    }
		open();
		if(optind==argc)
		    {
		    igzstreambuf buf;
		    istream in(&buf);
		    this->run("<stdin>",in);
		    }
		else
		    {
		    while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			char* input=argv[optind++];
			igzstreambuf buf(input);
			istream in(&buf);
			this->run(input,in);
			buf.close();
			}
		    }
		close();
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
    {
    VcfToSqlite app;
    return app.main(argc,argv);
    }
