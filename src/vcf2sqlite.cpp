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
#include "numeric_cast.h"
#define NOWHERE
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
	auto_ptr<Statement> insert_vcfmeta;
	auto_ptr<Statement> insert_sample;
	auto_ptr<Statement> select_sample;
	auto_ptr<Statement> insert_variation;
	auto_ptr<Statement> select_variation;
	auto_ptr<Statement> insert_vcfrow;
	auto_ptr<Statement> insert_vcfrowinfo;
	auto_ptr<Statement> insert_vcfcall;

	VcfToSqlite():dataasename(NULL)
	    {

	    }

	void close()
	    {
	    this->connection->execute("COMMIT");
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
	    this->connection->execute(
	       "create table if not exists VCFMETA("
		COLUMN_ID
		COLUMN_CREATED
		"vcffile_id INT NOT NULL REFERENCES VCFFILE(id) ON DELETE CASCADE,"
		"nLine INT NOT NULL DEFAULT -1,"
		"value TEXT NOT NULL "
	       ")");
	    insert_vcfmeta = this->connection->prepare(
	       "insert into VCFMETA(vcffile_id,nLine,value) values (?,?,?)"
		);
	    /* SAMPLES */
	    this->connection->execute(
		   "create table if not exists SAMPLE("
		    COLUMN_ID
		    COLUMN_CREATED
		    "name TEXT NOT NULL UNIQUE"
		   ")");
	    select_sample = this->connection->prepare(
	       "select id from SAMPLE where name=?"
		);
	    insert_sample = this->connection->prepare(
	       "insert into SAMPLE(name) values (?)"
		);
	    /* VARIATIONS */
	    this->connection->execute(
	       "create table if not exists VARIATION("
		COLUMN_ID
		COLUMN_CREATED
		"CHROM VARCHAR(20) NOT NULL,"
		"POS INT NOT NULL,"
		"RS VARCHAR(50),"
		"REF VARCHAR(10) NOT NULL,"
		"ALT VARCHAR(50) NOT NULL"
	       ")");
	    /*
	    this->connection->execute(
	       "create UNIQUE INDEX IF NOT EXISTS VAR1 ON "
	       "VARIATION(CHROM,POS,REF,ALT)"
		);
	     */
	    select_variation = this->connection->prepare(
	       "select id from VARIATION where (CHROM=? AND POS=? AND REF=? AND ALT=?)"
		);
	    insert_variation = this->connection->prepare(
	       "insert into VARIATION(CHROM,POS,RS,REF,ALT) values (?,?,?,?,?)"
		);
	    /* VCFROW */
	    this->connection->execute(
	       "create table if not exists VCFROW("
		COLUMN_ID
		COLUMN_CREATED
		"nLine INT NOT NULL DEFAULT -1,"
		"vcffile_id INT NOT NULL REFERENCES VCFFILE(id) ON DELETE CASCADE,"
		"variation_id INT NOT NULL REFERENCES VARIATION(id) ON DELETE CASCADE,"
		"QUAL FLOAT,"
		"FILTER TEXT"
	       ")");
	    insert_vcfrow = this->connection->prepare(
	       "insert into VCFROW(nLine,vcffile_id,variation_id,QUAL,FILTER) values (?,?,?,?,?)"
		);
	    /** VCFROWINFO */
	    this->connection->execute(
	       "create table if not exists VCFROWINFO("
		COLUMN_ID
		//COLUMN_CREATED
		"nIndex INT NOT NULL DEFAULT -1,"
		"vcfrow_id INT NOT NULL REFERENCES VCFROW(id) ON DELETE CASCADE,"
		"prop VARCHAR(10) NOT NULL,"
		"value VARCHAR(50)"
	       ")");
	    insert_vcfrowinfo = this->connection->prepare(
	       "insert into VCFROWINFO(nIndex,vcfrow_id,prop,value) values (?,?,?,?)"
		);
	    /** VCFCALL */
	    this->connection->execute(
	       "create table if not exists VCFCALL("
		COLUMN_ID
		//COLUMN_CREATED
		"nIndex INT NOT NULL DEFAULT -1,"
		"vcfrow_id INT NOT NULL REFERENCES VCFROW(id) ON DELETE CASCADE,"
		"sample_id INT NOT NULL REFERENCES SAMPLE(id) ON DELETE CASCADE,"
		"prop VARCHAR(10) NOT NULL,"
		"value VARCHAR(30)"
	       ")");
	    insert_vcfcall = this->connection->prepare(
	       "insert into VCFCALL(nIndex,vcfrow_id,sample_id,prop,value) values (?,?,?,?,?)"
		);

	    this->connection->execute("BEGIN");
	    }


	int64_t get_variation_id(
		const char* chrom,
		int64_t pos,
		const char* rs,
		const char* ref,
		const char* alt
		)
	    {
	    int64_t id=-1L;
	    select_variation->reset();
	    select_variation->bind_string(1,chrom);
	    select_variation->bind_int(2,pos);
	    select_variation->bind_string(3,ref);
	    select_variation->bind_string(4,alt);

	    while(select_variation->step()!=Statement::DONE)
		{
		id=select_variation->get_int64(1);
		WHERE("got variation id:"<< id);
		break;
		}
	    if(id==-1L)
		{
		insert_variation->reset();
		insert_variation->bind_string(1,chrom);
		insert_variation->bind_int(2,pos);
		if(rs==0 || strlen(rs)==0 || strcmp(rs,".")==0)
		    {
		    insert_variation->bind_null(3);
		    }
		else
		    {
		    insert_variation->bind_string(3,rs);
		    }
		insert_variation->bind_string(4,ref);
		insert_variation->bind_string(5,alt);
		insert_variation->execute();
		id= connection->last_insert_id();
		WHERE("created variation id:"<< id);
		}
	    return id;
	    }

	int64_t get_sample_id_by_name(const char* name)
	    {
	    int64_t id=-1L;
	    select_sample->reset();
	    select_sample->bind_string(1,name);
	    while(select_sample->step()!=Statement::DONE)
		{
		id=select_sample->get_int64(1);
		break;
		}
	    if(id==-1L)
		{
		insert_sample->reset();
		insert_sample->bind_string(1,name);
		insert_sample->execute();
		id= connection->last_insert_id();
		}
	    return id;
	    }


	void run(const char* filename,std::istream& in)
	    {
	    Tokenizer tab('\t');
	    int count_meta=0;
	    int count_vcfrow=0;
	    int state=0;
	    insert_vcffile->bind_string(1,filename);
	    insert_vcffile->execute();
	    int64_t vcf_file_id= connection->last_insert_id();
	    WHERE("VCF id="<< vcf_file_id );
	    vector<int64_t> samples_ids;
	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(state==0 &&
		    line.size()>1 &&
		    line[0]=='#' &&
		    line[1]=='#')
		    {
		    if(line.size()==2) continue;
		    insert_vcfmeta->bind_int(1,vcf_file_id);
		    insert_vcfmeta->bind_int(2,++count_meta);
		    line=line.substr(2);
		    insert_vcfmeta->bind_string(3,line.c_str());
		    insert_vcfmeta->execute();
		    insert_vcfmeta->reset();
		    }
		else if(state==0 &&
			line.size()>0 &&
			line[0]=='#'
			)
		    {
		    state=1;
		    tab.split(line,tokens);
		    if(tokens.size()<10) THROW("Expected at least 10 columns in " << line);
		    if(tokens[0].compare("#CHROM")!=0) THROW("Expected '#CHROM' in column 1 of " << line);
		    if(tokens[1].compare("POS")!=0) THROW("Expected 'POS' in column 2 of " << line);
		    if(tokens[2].compare("ID")!=0) THROW("Expected 'ID' in column 3 of " << line);
		    if(tokens[3].compare("REF")!=0) THROW("Expected 'REF' in column 4 of " << line);
		    if(tokens[4].compare("ALT")!=0) THROW("Expected 'ALT' in column 5 of " << line);
		    if(tokens[5].compare("QUAL")!=0) THROW("Expected 'QUAL' in column 6 of " << line);
		    if(tokens[6].compare("FILTER")!=0) THROW("Expected 'FILTER' in column 7 of " << line);
		    if(tokens[7].compare("INFO")!=0) THROW("Expected 'INFO' in column 8 of " << line);
		    if(tokens[8].compare("FORMAT")!=0) THROW("Expected 'FORMAT' in column 9 of " << line);
		    for(size_t i=9;i< tokens.size();++i)
			{
			samples_ids.push_back(get_sample_id_by_name(tokens[i].c_str()));
			}
		    }
		else if(state==1 && !line.empty() && line[0]!='#')
		    {
		    tab.split(line,tokens);
		    if(tokens.size()!=9+samples_ids.size())
			{
			THROW("Expected " << (9+samples_ids.size()) << " in "<< line);
			}
		    int64_t position;
		    if(!numeric_cast(tokens[1].c_str(),&position))
			{
			THROW("Not a position in " << line);
			}
		    int64_t variation_id= get_variation_id(
			tokens[0].c_str(),
			position,
			tokens[2].c_str(),
			tokens[3].c_str(),
			tokens[4].c_str()
			);
		    insert_vcfrow->reset();
		    insert_vcfrow->bind_int(1,++count_vcfrow);
		    insert_vcfrow->bind_int(2,vcf_file_id);
		    insert_vcfrow->bind_int(3,variation_id);
		    insert_vcfrow->bind_string(4,tokens[5].c_str());
		    insert_vcfrow->bind_string(5,tokens[6].c_str());
		    insert_vcfrow->execute();
		    int64_t vcfrow_id= connection->last_insert_id();

		    //cerr << count_vcfrow << endl;

		    Tokenizer semicolon(';');
		    vector<string> infos;
		    semicolon.split(tokens[7],infos);
		    for(size_t j=0;j< infos.size();++j)
			{
			string::size_type eq=infos[j].find_first_of('=');


			insert_vcfrowinfo->reset();
			insert_vcfrowinfo->bind_int(1,(int64_t)j);
			insert_vcfrowinfo->bind_int(2,vcfrow_id);
			if(eq==std::string::npos)
			    {
			    insert_vcfrowinfo->bind_string(3,infos[j].c_str());
			    insert_vcfrowinfo->bind_null(4);
			    }
			else
			    {
			    string k=infos[j].substr(0,eq);
			    insert_vcfrowinfo->bind_string(3,k.c_str());
			    k=infos[j].substr(eq+1);
			    insert_vcfrowinfo->bind_string(4,k.c_str());
			    }
			insert_vcfrowinfo->execute();
			}
		    Tokenizer colon(':');
		    vector<string> format;

		    colon.split(tokens[8],format);
		    for(size_t j=0;j< samples_ids.size();++j)
			{
			vector<string> call;
			colon.split(tokens[9+j],call);
			if(call.size()!=format.size())
			    {
			    THROW("size(FORMAT)!=size(call) in "<< line);
			    }
			for(size_t k=0;k< format.size();++k)
			    {
			    insert_vcfcall->reset();
			    insert_vcfcall->bind_int(1,(int32_t)k);
			    insert_vcfcall->bind_int(2,vcfrow_id);
			    insert_vcfcall->bind_int(3,samples_ids[j]);
			    insert_vcfcall->bind_string(4,format[k].c_str());
			    insert_vcfcall->bind_string(5,call[k].c_str());
			    insert_vcfcall->execute();
			    }
			}

		    }
		else
		    {
		    THROW("BAD input "<< line);
		    }
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
