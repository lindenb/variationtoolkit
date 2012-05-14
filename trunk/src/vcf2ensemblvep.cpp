/*
 * vcf2sqlite.cpp
 *
 *  Created on: Dec 25, 2011
 *      Author: lindenb
 */
#include <iostream>
#include <cstdlib>

#ifndef NOSQLITE

#include <string>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>

#include "xsqlite.h"
#include "application.h"
#include "throw.h"
#include "zstreambuf.h"
#include "numeric_cast.h"
#include "where.h"

/*

3PRIME_UTR
5PRIME_UTR
CODING_UNKNOWN
COMPLEX_INDEL
DOWNSTREAM
ESSENTIAL_SPLICE_SITE
FRAMESHIFT_CODING
INTRONIC
NMD_TRANSCRIPT
NON_SYNONYMOUS_CODING
PARTIAL_CODON
SPLICE_SITE
STOP_GAINED
STOP_LOST
SYNONYMOUS_CODING
UPSTREAM
WITHIN_MATURE_miRNA
WITHIN_NON_CODING_GEN

*/

using namespace std;

#define TABLE "vep" /* variant effect prediction */


#define ECHO_EMPTY_LINE	cout << line ;\
for(size_t i=0;i< cols.size();i++)\
    {\
    if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())\
	{\
	continue;\
	}\
    cout << "\t.";\
    }\
cout << endl;

#define CONSEQUENCE_COLUMN 6

class VcfEnsemblPredictionFilter:public AbstractApplication
    {
    public:
	char* databasename;
	auto_ptr<Connection> connection;
	auto_ptr<Statement> select_variation;
	vector<string> cols;
	set<string> limitCols;
	int32_t refColumn;
	int32_t altColumn;

	VcfEnsemblPredictionFilter():databasename(NULL),refColumn(3),altColumn(4)
	    {
	    cols.push_back("Uploaded_variation");
	    cols.push_back("Location");
	    cols.push_back("Allele");
	    cols.push_back("Gene");
	    cols.push_back("Feature");
	    cols.push_back("Feature_type");
	    cols.push_back("Consequence");//6 CONSEQUENCE_COLUMN
	    cols.push_back("cDNA_position");
	    cols.push_back("CDS_position");
	    cols.push_back("Protein_position");
	    cols.push_back("Amino_acids");
	    cols.push_back("Codons");
	    cols.push_back("Existing_variation");
	    cols.push_back("Extra");
	    }

	void close()
	    {
	    }

	void open()
	    {
	    if(databasename==0) THROW("DB name undefined.");
	    ConnectionFactory cf;
	    cf.set_allow_create(false);
	    cf.set_read_only(true);
	    cf.set_filename(databasename);
	    this->connection=cf.create();



	    ostringstream os;
	    os << "select ";
	    for(size_t i=0;i< cols.size();++i)
		{
		if(i>0) os << ",";
		os << cols[i];
		}
	    os << " from " TABLE " where Uploaded_variation=?";
	    string sql(os.str());
	    select_variation = this->connection->prepare(sql.c_str());
	    }





	void run(std::istream& in)
	    {
	    Tokenizer tab('\t');
	    Tokenizer comma(',');
	    vector<string> tokens;
	    string line;
	    while(getline(in,line,'\n'))
		{
		if(line.empty()) continue;
		if(line[0]=='#')
		    {
		    cout << line;
		    for(size_t i=0;i< cols.size();i++)
			{
			if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())
			    {
			    continue;
			    }
			cout << "\t";
			cout << "evp." << cols[i];
			}
		    cout << endl;
		    continue;
		    }




		tab.split(line,tokens);
		if(tokens.size()<2)
		    {
		    THROW("Expected 2 columns in "<< line);
		    }

		if( (int)tokens.size()<=refColumn)
		    {
		    THROW("cannot get the ALT column in "<< line);
		    }

		if( (int)tokens.size()<=altColumn)
		    {
		    THROW("cannot get the REF column in "<< line);
		    }

		int32_t position;
		if(!numeric_cast<int32_t>(tokens[1].c_str(),&position))
		    {
		    THROW("Not a position in " << line);
		    }
		std::transform(tokens[refColumn].begin(),tokens[refColumn].end(),tokens[refColumn].begin(),(int(*)(int))toupper);
		std::transform(tokens[altColumn].begin(),tokens[altColumn].end(),tokens[altColumn].begin(),(int(*)(int))toupper);

		ostringstream os;
		bool found=false;
		os << tokens[0] << "_" << position << "_" << tokens[refColumn] << "/" << tokens[altColumn];
		string variation(os.str());
		auto_ptr<string> ret(0);
		select_variation->reset();
		select_variation->bind_string(1,variation.c_str());

		for(;;)
		    {

		    if(select_variation->step()==Statement::DONE)
			{

			break;
			}


		    found=true;
		    cout << line ;



		    for(size_t i=0;i< cols.size();i++)
			{
			if(!limitCols.empty() && limitCols.find(cols[i])==limitCols.end())
			    {
			    continue;
			    }
			cout << "\t";
			const char* val =select_variation->get_string(i+1);
			if(val==0 || val[0]==0)
			    {
			    cout << ".";
			    }
			else
			    {
			    cout << val;
			    }
			}
		    cout << endl;
		    }


		if(!found)
		    {
		    ECHO_EMPTY_LINE
		    }

		}



	    }


	virtual void usage(ostream& out,int argc,char** argv)
	    {
	    out << argv[0] << " Pierre Lindenbaum PHD. 2012.\n";
	    out << VARKIT_REVISION << endl;
	    out << "Append EVS data to VCF like file from a sqlite database.\n";
	    out << "Compilation: "<<__DATE__<<"  at "<< __TIME__<<".\n";
	    out << "Options:\n";
	    out << "  -f (file) sqlite filename. (REQUIRED).\n";
	    out << "  -c (column-name) limit header to this column name (can be used multiple times).\n";
	    out << "(stdin|vcf|vcf.gz)\n\n";
	    out << endl;
	    out << "Building the SQLITE table:"<< endl;
	    out << " sqlite> create table " TABLE "(";
	    for(size_t i=0;i< cols.size();++i)
		{
		if(i>0) out << ",";
		out << cols[i] << " TEXT NOT NULL";
		}
	    out << ");\n";
	    out << " sqlite> create index variationidx on "TABLE"(Uploaded_variation);\n";
	    out << " sqlite> .separator \"\\t\"\n";
	    out << " sqlite> .import \"input.evp.tsv\" "TABLE";\n";

	    out << endl;
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
			databasename=argv[++optind];
			}
		    else if(strcmp(argv[optind],"-c")==0 && optind+1<argc)
			{
			limitCols.insert((argv[++optind]));
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
		if(databasename==NULL)
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
		    this->run(in);
		    }
		else
		    {
		    while(optind< argc)
			{
			if(AbstractApplication::stopping()) break;
			char* input=argv[optind++];
			igzstreambuf buf(input);
			istream in(&buf);
			this->run(in);
			buf.close();
			}
		    }
		close();
		return EXIT_SUCCESS;
		}
    };


int main(int argc,char** argv)
    {
    VcfEnsemblPredictionFilter app;
    return app.main(argc,argv);
    }

#else

int main(int argc,char** argv)
    {
    std::cerr << argv[0] << " was compiled without sqlite3 ( $SQLITE_LIB undefined) \n";
    return EXIT_FAILURE;
    }

#endif
